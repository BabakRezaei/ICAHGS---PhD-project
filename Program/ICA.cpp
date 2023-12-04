#include "ICA.h"
#include <math.h>
#include <ctime>
// #include <omp.h>

// Create initial countries
void ICA::CreateCountries()
{	
	int counter = 0;
	int attempt = 0;
	std::vector < double > empiresCosts;
	while (counter < params->NumOfInitialImperialists)
	{
		attempt++;
		std::printf("atempt %d, ", attempt);
		Country * randCountry = new Country(params);
		Empire * empire = new Empire(params->Zeta);
		split->generalSplit(randCountry, params->nbVehicles);
		localSearch->run(randCountry, params->penaltyCapacity, params->penaltyDuration);
		if (randCountry->isFeasible)
		{
			empire->Imperialist = randCountry;
			empire->population = new Population(params, split, localSearch);
			empire->population->addCountry(randCountry, true);
			Empires.push_back(empire);
			empiresCosts.push_back(empire->Imperialist->myCostSol.penalizedCost);
			counter++;
			attempt = 0;
			std::printf("%1d empire is generated\n", counter);
		}
		else
		{
			localSearch->run(randCountry, params->penaltyCapacity*10., params->penaltyDuration*10.);
			if (randCountry->isFeasible)
			{
				empire->Imperialist = randCountry;
				empire->population = new Population(params, split, localSearch);
				empire->population->addCountry(randCountry, true);
				Empires.push_back(empire);
				empiresCosts.push_back(empire->Imperialist->myCostSol.penalizedCost);
				counter++;
				attempt = 0;
				std::printf("%1d empire is generated\n", counter);
			}
		}
	}

	double sumEmpiresCosts = 0;
	double empPower = 1.3 * *std::max_element(empiresCosts.begin(), empiresCosts.end());
	for (int i = 0 ; i < params->NumOfInitialImperialists ; i++)
	{
		empiresCosts[i] = empPower - empiresCosts[i];
		sumEmpiresCosts += empiresCosts[i];
	}
	
	int sumOfColonies = 0;
	std::vector < int > NumOfClonies;
	for (int i = 0; i < params->NumOfInitialImperialists ; i++)
	{
		int NumOfCol = std::round(empiresCosts[i] / sumEmpiresCosts * params->NumOfAllColonies);
		if (NumOfCol == 0) NumOfCol = 1;
		sumOfColonies += NumOfCol;
		NumOfClonies.push_back(NumOfCol);
	}
	int diffSum = sumOfColonies - params->NumOfAllColonies;
	if (diffSum <= 0) NumOfClonies[0] -= diffSum;
	else NumOfClonies[params->NumOfInitialImperialists - 1] -= diffSum;

	for (int i = 0 ; i < (int)Empires.size() ; i++)
	{
		for (int j = 0 ; j < NumOfClonies[i] ; j++)
		{
			Country * cntry = new Country(params);
			split->generalSplit(cntry, params->nbVehicles);
			localSearch->run(cntry, params->penaltyCapacity, params->penaltyDuration);
			bool isNewBest = Empires[i]->population->addCountry(cntry, true);
			if (!cntry->isFeasible && std::rand() % 2 == 0)  // Repair half of the solutions in case of infeasibility
			{
				localSearch->run(cntry, params->penaltyCapacity*10., params->penaltyDuration*10.);
				if (cntry->isFeasible)
				{
					isNewBest = Empires[i]->population->addCountry(cntry, false) || isNewBest;
				}
			}
			if (isNewBest) Empires[i]->Imperialist = cntry;
		}
		Empires[i]->colonySize = NumOfClonies[i];
		Empires[i]->calculateCost();
	}
}

// Mixed Assimilation and Revolution
void ICA::AssimRevolve(bool changeAssimilationType, int decade, int nbIter)
{	
	for (Empire * empire : Empires)
	{
		// srand(empire->randomseed);
		int numRevolvColonies = std::round(empire->colonySize * params->RevolutionRate);
		if (numRevolvColonies == 0) numRevolvColonies = 1;

		for (int i = 0 ; i < numRevolvColonies ; i++)
		{
			Country * newColony = new Country(params);

			if (empire->AssimType) // do assimilation between imperialist and colonies
			{
				if (decade == 0) mycrossoverOX(newColony, empire->Imperialist, empire->population->getBinaryTournament());
				else mycrossoverOX(newColony, bestSolutionOverall, empire->population->getBinaryTournament());
			}
			else // do assimilation between colonies
				mycrossoverOX(newColony, empire->population->getBinaryTournament(), empire->population->getBinaryTournament());
			
			localSearch->run(newColony, params->penaltyCapacity, params->penaltyDuration);
			bool isNewBest = empire->population->addCountry(newColony, true);
			if (!newColony->isFeasible && std::rand()%2 == 0) // Repair half of the solutions in case of infeasibility
			{
				localSearch->run(newColony, params->penaltyCapacity*10., params->penaltyDuration*10.);
				if (newColony->isFeasible) isNewBest = (empire->population->addCountry(newColony, false) || isNewBest);
			}
			if (isNewBest)
			{
				empire->Imperialist = newColony;
				empire->lastResetDecade = decade;
				empire->nextRestartDecade = 0;
			}
			// if (newColony->isFeasible)
			// {
			// 	if ((int)empire->population->feasibleSubpopulation.size() > 1)
			// 		empire->population->removeWorstBiasedFitness(empire->population->feasibleSubpopulation);
			// }
			// else
			// {
			// 	if ((int)empire->population->infeasibleSubpopulation.size() > 1)
			// 		empire->population->removeWorstBiasedFitness(empire->population->infeasibleSubpopulation);
			// }
			empire->managePenaltyCounter++;
			if (empire->managePenaltyCounter % 100 == 0) empire->population->managePenalties();
			empire->nextRestartDecade++;
			if (empire->nextRestartDecade == nbIter) {RestartEmpires(empire); empire->nextRestartDecade = 0;}
			
			// changeAssimilationType = !changeAssimilationType;
		}
		empire->AssimType = false;
	}
}

// Crossover function
void ICA::mycrossoverOX(Country * result, const Country * country1, const Country * country2)
{
	// Frequency table to track the customers which have been already inserted
	std::vector <bool> freqClient = std::vector <bool> (params->nbClients + 1, false);

	// Picking the beginning and end of the crossover zone
	int start = std::rand() % params->nbClients;
	int end = std::rand() % params->nbClients;
	while (end == start) end = std::rand() % params->nbClients;

	// Copy in place the elements from start to end (possibly "wrapping around" the end of the array)
	int j = start;
	while (j % params->nbClients != (end + 1) % params->nbClients)
	{
		result->chromT[j % params->nbClients] = country1->chromT[j % params->nbClients];
		freqClient[result->chromT[j % params->nbClients]] = true;
		j++;
	}

	// Fill the remaining elements in the order given by the second parent
	for (int i = 1; i <= params->nbClients; i++)
	{
		int temp = country2->chromT[(end + i) % params->nbClients];
		if (freqClient[temp] == false)
		{
			result->chromT[j % params->nbClients] = temp;
			j++;
		}
	}
	split->generalSplit(result, country1->myCostSol.nbRoutes);
}

// Imperialistic competition
void ICA::ImperialisticCompetition()
{
	if (Empires.size() > 1)
	{
		Empire * WeakestEmpire = new Empire(params->Zeta);
		Empire * SelectedEmpire = new Empire(params->Zeta);

		std::vector < double > ImperialistCosts;
		std::vector < int > colonyNumbers;
		std::vector < double > clocks;

		for (Empire * empire : Empires)
		{
			ImperialistCosts.push_back(empire->Imperialist->myCostSol.penalizedCost);
			colonyNumbers.push_back(empire->colonySize);
			clocks.push_back(empire->population->bestfoundclock);
		}

		int maxImpCostIndex = std::max_element(ImperialistCosts.begin(),ImperialistCosts.end()) - ImperialistCosts.begin();
		int minImpCostIndex = std::min_element(ImperialistCosts.begin(),ImperialistCosts.end()) - ImperialistCosts.begin();

		if (maxImpCostIndex == minImpCostIndex)
		{
			// it means all empires have same imperialist cost
			// we select the empire with larger colony number as selected and next index as weakest
			// int maxColonyNumIndex = std::max_element(colonyNumbers.begin(),colonyNumbers.end()) - colonyNumbers.begin();

			// we select the empire with minimum best found clcok then select the next index as the weakest
			int minBestClock = std::min_element(clocks.begin(),clocks.end()) - clocks.begin();

			minImpCostIndex = minBestClock;
			maxImpCostIndex = (minImpCostIndex + 1) % (int)Empires.size();
		}
		
		WeakestEmpire = Empires[maxImpCostIndex];
		int SelectedEmpireIndex = 0;
		if ((int)Empires.size() == 2) SelectedEmpireIndex = (maxImpCostIndex + 1) % (int)Empires.size();
		else
		{
			SelectedEmpireIndex = std::rand() % (int)Empires.size();
			while(SelectedEmpireIndex == maxImpCostIndex) SelectedEmpireIndex = std::rand() % (int)Empires.size();
		}
		SelectedEmpire = Empires[SelectedEmpireIndex];

		SelectedEmpire->population->addCountry(WeakestEmpire->population->getRandomCountry(), true);
		SelectedEmpire->colonySize++;
		WeakestEmpire->colonySize--;


		if (WeakestEmpire->colonySize == 0)
		{
			SelectedEmpire->population->addCountry(WeakestEmpire->Imperialist, true);
			SelectedEmpire->colonySize++;
			std::rotate(Empires.begin(), Empires.begin() + maxImpCostIndex + 1, Empires.end());
			Empires.pop_back();
			std::rotate(Empires.begin(), Empires.begin() + (int)Empires.size() - maxImpCostIndex, Empires.end());
		}
		else WeakestEmpire->calculateCost();

		SelectedEmpire->calculateCost();
	}
}

// Creating new imperialist and countries after reseting an empire
void ICA::CreateNewCountries(Empire * empire)
{
	std::cout << "----- GENERATING NEW EMPIRE (Totally)" << std::endl;
	int counter = 0;
	while (counter != 1)
	{
		Country * randCountry = new Country(params);
		split->generalSplit(randCountry, params->nbVehicles);
		localSearch->run(randCountry, params->penaltyCapacity, params->penaltyDuration);
		if (randCountry->isFeasible)
		{
			empire->Imperialist = randCountry;
			empire->population->restart();
			empire->population->addCountry(randCountry, true);
			counter++;
		}
		else
		{
			localSearch->run(randCountry, params->penaltyCapacity*10., params->penaltyDuration*10.);
			if (randCountry->isFeasible)
			{
				empire->Imperialist = randCountry;
				empire->population->restart();
				empire->population->addCountry(randCountry, true);
				counter++;
			}
		}
	}

	for (int j = 0 ; j < empire->colonySize ; j++)
	{
		Country * cntry = new Country(params);
		split->generalSplit(cntry, params->nbVehicles);
		localSearch->run(cntry, params->penaltyCapacity, params->penaltyDuration);
		bool isNewBest = empire->population->addCountry(cntry, true);
		if (!cntry->isFeasible && std::rand() % 2 == 0)  // Repair half of the solutions in case of infeasibility
		{
			localSearch->run(cntry, params->penaltyCapacity*10., params->penaltyDuration*10.);
			if (cntry->isFeasible)
			{
				isNewBest = empire->population->addCountry(cntry, false) || isNewBest;
			}
		}
		if (isNewBest) empire->Imperialist = cntry;
	}
	empire->calculateCost();
}

void ICA::NewCreateNewCountries(Empire * empire)
{
	std::cout << "----- GENERATING NEW EMPIRE (Partially)" << std::endl;
	int sliceofColonysize = std::round(empire->colonySize / 4);
	// std::set < int > randomIndices;
	std::vector < int > randomIndices;
	std::vector < Country * > temp;

	int fromFeas = std::round(sliceofColonysize * 0.25);  // select 25% from feasible and 75% from infeasible
	
	for (int i = 0; i < fromFeas; i++)
	{
		int randindex = std::rand() % (int)empire->population->feasibleSubpopulation.size();
		temp.push_back(empire->population->feasibleSubpopulation[randindex]);
	}
	for (int i = 0; i < (sliceofColonysize - fromFeas); i++)
	{
		int randindex = std::rand() % (int)empire->population->infeasibleSubpopulation.size();
		temp.push_back(empire->population->infeasibleSubpopulation[randindex]);
	}

	empire->population->restart();

	for (Country * cntry : temp)
	{
		empire->population->addCountry(cntry, true);
	}

	for (int j = 0 ; j < (empire->colonySize -  sliceofColonysize) ; j++)
	{
		Country * cntry = new Country(params);
		split->generalSplit(cntry, params->nbVehicles);
		localSearch->run(cntry, params->penaltyCapacity, params->penaltyDuration);
		bool isNewBest = empire->population->addCountry(cntry, true);
		if (!cntry->isFeasible && std::rand() % 2 == 0)  // Repair half of the solutions in case of infeasibility
		{
			localSearch->run(cntry, params->penaltyCapacity*10., params->penaltyDuration*10.);
			if (cntry->isFeasible)
			{
				isNewBest = empire->population->addCountry(cntry, false) || isNewBest;
			}
		}
		if (isNewBest) empire->Imperialist = cntry;
	}
	empire->calculateCost();
}

// Restarting passed Empire and generating new Imperialist and colonies
void ICA::RestartEmpires(Empire * empire)
{
	// while (!Empires.empty()) Empires.pop_back();
	std::cout << "----- GENERATING NEW EMPIRE" << std::endl;
	if (empire->Imperialist->myCostSol.penalizedCost == empire->LastResetValue)
	{
		empire->NumOfResets++;
	}
	else
	{
		empire->LastResetValue = empire->Imperialist->myCostSol.penalizedCost;
		empire->NumOfResets = 1;
	}
	if (empire->NumOfResets == 2)
	{
		if (empire->Imperialist->myCostSol.penalizedCost != bestSolutionOverall->myCostSol.penalizedCost) empire->AssimType = true;
		else {empire->AssimType = false; }  // empire->NumOfResets = 0;
		CreateNewCountries(empire);  // reset colonies totally
		empire->NumOfResets++;
	}
	else if (empire->NumOfResets > 2)
	{
		CreateNewCountries(empire);  // reset colonies totally
		empire->NumOfResets++;
		// empire->NumOfResets = 0;
		empire->AssimType = false;
	}
	else NewCreateNewCountries(empire);  // reset colonies partially

	std::printf("Empire cost: %.2f, Imperialist cost: %.2f, Number of colonies: %3d\n", empire->cost, empire->Imperialist->myCostSol.penalizedCost, empire->colonySize);
	std::printf("----- NEW GENERATED EMPIRE\n");
}

// *********************************************************************

// preparing initial countries and run the ICA
void ICA::run(int Decades, int timeLimit, int CompetitionPeriod, int seed, int bks, int nbIter, int prevtime)
{	
	double bestFoundCost = 1.e30;

	std::cout << "----- GENERATING INITIAL COUNTRIES" << std::endl;

	CreateCountries();

	std::cout << "----- STARTING ICA ALGORITHM" << std::endl;
	
	int decade = 0;
	
	bool changeAssimilationType = false;

	// Main Loop
	while (Empires.size() >= 1 && clock()/CLOCKS_PER_SEC < timeLimit && bestFoundCost >= bks)
	{	
		if (decade % 100 == 0)
        {
            for (Empire * empire : Empires)
            {
                double clk = (double)(empire->population->bestfoundclock) / (double)CLOCKS_PER_SEC -(double)prevtime;
               
                std::printf("Gener: %4d, ", decade);

                if (empire->population->getBestFound() != NULL)
                {
                    std::printf("Best cost: %.2f ", empire->population->getBestFound()->myCostSol.penalizedCost);
                    // cost1 = empire->population->getBestFound()->myCostSol.penalizedCost;
                }
                else std::printf("Best cost: NO-BEST ");

                std::printf("in %0.2f sec, IndNo: %3d | ", clk, empire->colonySize);

                if (empire->population->getBestFeasible() != NULL)
                {
                    std::printf("Best feasible: %.2f, ", empire->population->getBestFeasible()->myCostSol.penalizedCost);
                    // cost2 = empire->population->getBestFeasible()->myCostSol.penalizedCost;
                }
                else std::printf("Best feasible: NO-FEAS, ");

                std::printf("iter to reset: %d, elapsed: %.2f\n", empire->nextRestartDecade, (double)clock() / (double)CLOCKS_PER_SEC -(double)prevtime);
                
            }
        }

		// Mixed Assimilation and Revolve colonies using Revolution rate
		AssimRevolve(changeAssimilationType, decade, nbIter);
		// ParallelAssimRevolve(changeAssimilationType, decade);

		if (decade % CompetitionPeriod == 0)
		{
			// Imperialistic competition
			ImperialisticCompetition();
		}

		for (Empire * empire : Empires)
		{
			if (empire->population->getBestFound()->myCostSol.penalizedCost < bestFoundCost)
			{
				bestFoundCost = empire->population->getBestFound()->myCostSol.penalizedCost;
				bestSolutionOverall = empire->population->getBestFound();
			}
		}
		
		decade++;
	}

	std::printf("----------------------------------------------\n");
	std::printf("finished at Decade: %4d\n", decade - 1);
	std::printf("Remained empires after finishing the algorithm:\n");
	for (Empire * empire : Empires)
    {
        // double cost = empire->population->getBestFound()->myCostSol.penalizedCost;
        if (empire->population->getBestFound() != NULL)
        {
            double clk = (double)(empire->population->bestfoundclock) / (double)CLOCKS_PER_SEC -(double)prevtime;
            std::printf("Best island solution cost: %.6f, Best found time: %.2f seconds, Number of indivs: %3d\n",
                        empire->population->getBestFound()->myCostSol.penalizedCost, clk, empire->colonySize);
        }
        else
        {
            std::printf("Best island solution cost: NO-BEST, Best found time: 0.00 seconds, Number of indivs: %3d\n",
                        empire->colonySize);
        }
    }
}

// returns the best found solution
Country * ICA::getBestFound()
{
	if (bestSolutionOverall->myCostSol.penalizedCost < 1.e29) return bestSolutionOverall;
	else return NULL;
}

// ICA initialization
ICA::ICA(Params * params, Split * split, LocalSearch * localSearch) : params(params), split(split), localSearch(localSearch)
{
	std::cout << "----- ICA ALGORITHM INITIALIZED -----" << std::endl;
}

// ICA destructor
ICA::~ICA(void)
{
	for (int i = 0 ; i < (int)Empires.size() ; i++) delete Empires[i];
	Empires.clear();
}
