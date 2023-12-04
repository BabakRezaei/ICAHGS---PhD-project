#include "Population.h"
#include "iomanip"

void Population::generatePopulation()
{
	for (int i = 0; i < 4*params->mu; i++)
	{
		Country * randomCntry = new Country(params);
		split->generalSplit(randomCntry, params->nbVehicles);
		localSearch->run(randomCntry, params->penaltyCapacity, params->penaltyDuration);
		addCountry(randomCntry, true);
		if (!randomCntry->isFeasible && std::rand() % 2 == 0)  // Repair half of the solutions in case of infeasibility
		{
			localSearch->run(randomCntry, params->penaltyCapacity*10., params->penaltyDuration*10.);
			if (randomCntry->isFeasible) addCountry(randomCntry, false);
		}
		delete randomCntry;
	}
}

bool Population::addCountry(const Country * cntry, bool updateFeasible)
{
	if (updateFeasible)
	{
		listFeasibilityLoad.push_back(cntry->myCostSol.capacityExcess < MY_EPSILON);
		listFeasibilityDuration.push_back(cntry->myCostSol.durationExcess < MY_EPSILON);
		listFeasibilityLoad.pop_front();
		listFeasibilityDuration.pop_front();
	}

	// Find the adequate subpopulation in relation to the country feasibility
	SubPopulation & subpop = (cntry->isFeasible) ? feasibleSubpopulation : infeasibleSubpopulation;

	// Create a copy of the country and updade the proximity structures calculating inter-country distances
	Country * myCountry = new Country(*cntry);
	for (Country * myCountry2 : subpop)
	{
		double myDistance = myCountry->brokenPairsDistance(myCountry2);
		myCountry2->cntrysPerProximity.insert({ myDistance, myCountry });
		myCountry->cntrysPerProximity.insert({ myDistance, myCountry2 });
	}

	// Identify the correct location in the population and insert the country
	int place = (int)subpop.size();
	while (place > 0 && subpop[place - 1]->myCostSol.penalizedCost > cntry->myCostSol.penalizedCost - MY_EPSILON) place--;
	subpop.emplace(subpop.begin() + place, myCountry);

	// Trigger a survivor selection if the maximimum population size is exceeded
	if ((int)subpop.size() > params->mu + params->lambda)
		while ((int)subpop.size() > params->mu)
			removeWorstBiasedFitness(subpop);

	// Track best solution
	if (cntry->isFeasible && cntry->myCostSol.penalizedCost < bestSolutionRestart.myCostSol.penalizedCost - MY_EPSILON)
	{
		bestSolutionRestart = *cntry;
		if (cntry->myCostSol.penalizedCost < bestSolutionOverall.myCostSol.penalizedCost - MY_EPSILON)
		{
			bestSolutionOverall = *cntry;
			bestfoundclock = clock();
			searchProgress.push_back({bestfoundclock, bestSolutionOverall.myCostSol.penalizedCost });
		}
		return true;
	}
	else
		return false;
}

void Population::updateBiasedFitnesses(SubPopulation & pop)
{
	// Ranking the countrys based on their diversity contribution (decreasing order of distance)
	std::vector <std::pair <double, int> > ranking;
	for (int i = 0 ; i < (int)pop.size(); i++) 
		ranking.push_back({-pop[i]->averageBrokenPairsDistanceClosest(params->nbClose),i});
	std::sort(ranking.begin(), ranking.end());

	// Updating the biased fitness values
	if (pop.size() == 1) 
		pop[0]->biasedFitness = 0;
	else
	{
		for (int i = 0; i < (int)pop.size(); i++)
		{
			double divRank = (double)i / (double)(pop.size() - 1); // Ranking from 0 to 1
			double fitRank = (double)ranking[i].second / (double)(pop.size() - 1);
			if ((int)pop.size() <= params->nbElite) // Elite countrys cannot be smaller than population size
				pop[ranking[i].second]->biasedFitness = fitRank;
			else 
				pop[ranking[i].second]->biasedFitness = fitRank + (1.0 - (double)params->nbElite / (double)pop.size()) * divRank;
		}
	}
}

void Population::removeWorstBiasedFitness(SubPopulation & pop)
{
	updateBiasedFitnesses(pop);
	if (pop.size() <= 1) throw std::string("Eliminating the best country: this should not occur in HGS");

	Country * worstCountry = NULL;
	int worstCountryPosition = -1;
	bool isWorstCountryClone = false;
	double worstCountryBiasedFitness = -1.e30;
	for (int i = 1; i < (int)pop.size(); i++)
	{
		bool isClone = (pop[i]->averageBrokenPairsDistanceClosest(1) < MY_EPSILON); // A distance equal to 0 indicates that a clone exists
		if ((isClone && !isWorstCountryClone) || (isClone == isWorstCountryClone && pop[i]->biasedFitness > worstCountryBiasedFitness))
		{
			worstCountryBiasedFitness = pop[i]->biasedFitness;
			isWorstCountryClone = isClone;
			worstCountryPosition = i;
			worstCountry = pop[i];
		}
	}

	pop.erase(pop.begin() + worstCountryPosition); // Removing the country from the population
	for (Country * myCountry2 : pop) myCountry2->removeProximity(worstCountry); // Cleaning its distances from the other countrys in the population
	delete worstCountry; // Freeing memory
}

void Population::restart()
{
	// std::cout << "----- RESET: CREATING A NEW POPULATION -----" << std::endl;
	// for (Country * cntry : feasibleSubpopulation) delete cntry ;
	// for (Country * cntry : infeasibleSubpopulation) delete cntry;
	feasibleSubpopulation.clear();
	infeasibleSubpopulation.clear();
	bestSolutionRestart = Country();
	// needsRestart = false;
	// generatePopulation();
	listFeasibilityLoad = std::list<bool>(100, true);
	listFeasibilityDuration = std::list<bool>(100, true);
	// searchProgress.clear();
}

void Population::managePenalties()
{
	// Setting some bounds [0.1,1000] to the penalty values for safety
	double fractionFeasibleLoad = (double)std::count(listFeasibilityLoad.begin(), listFeasibilityLoad.end(), true) / (double)listFeasibilityLoad.size();
	if (fractionFeasibleLoad < params->targetFeasible - 0.05 && params->penaltyCapacity < 1000) params->penaltyCapacity = std::min<double>(params->penaltyCapacity * 1.2,1000.);
	else if (fractionFeasibleLoad > params->targetFeasible + 0.05 && params->penaltyCapacity > 0.1) params->penaltyCapacity = std::max<double>(params->penaltyCapacity * 0.85, 0.1);

	// Setting some bounds [0.1,1000] to the penalty values for safety
	double fractionFeasibleDuration = (double)std::count(listFeasibilityDuration.begin(), listFeasibilityDuration.end(), true) / (double)listFeasibilityDuration.size();
	if (fractionFeasibleDuration < params->targetFeasible - 0.05 && params->penaltyDuration < 1000)	params->penaltyDuration = std::min<double>(params->penaltyDuration * 1.2,1000.);
	else if (fractionFeasibleDuration > params->targetFeasible + 0.05 && params->penaltyDuration > 0.1) params->penaltyDuration = std::max<double>(params->penaltyDuration * 0.85, 0.1);

	// Update the evaluations
	for (int i = 0; i < (int)infeasibleSubpopulation.size(); i++)
		infeasibleSubpopulation[i]->myCostSol.penalizedCost = infeasibleSubpopulation[i]->myCostSol.distance
		+ params->penaltyCapacity * infeasibleSubpopulation[i]->myCostSol.capacityExcess
		+ params->penaltyDuration * infeasibleSubpopulation[i]->myCostSol.durationExcess;

	// If needed, reorder the countrys in the infeasible subpopulation since the penalty values have changed (simple bubble sort for the sake of simplicity)
	for (int i = 0; i < (int)infeasibleSubpopulation.size(); i++)
	{
		for (int j = 0; j < (int)infeasibleSubpopulation.size() - i - 1; j++)
		{
			if (infeasibleSubpopulation[j]->myCostSol.penalizedCost > infeasibleSubpopulation[j + 1]->myCostSol.penalizedCost + MY_EPSILON)
			{
				Country * cntry = infeasibleSubpopulation[j];
				infeasibleSubpopulation[j] = infeasibleSubpopulation[j + 1];
				infeasibleSubpopulation[j + 1] = cntry;
			}
		}
	}
}

Country * Population::getBinaryTournament ()
{
	Country * country1 ;
	Country * country2 ;

	updateBiasedFitnesses(feasibleSubpopulation);
	updateBiasedFitnesses(infeasibleSubpopulation);
	
	int place1 = std::rand() % (feasibleSubpopulation.size() + infeasibleSubpopulation.size()) ;
	if (place1 >= (int)feasibleSubpopulation.size()) country1 = infeasibleSubpopulation[place1 - feasibleSubpopulation.size()] ;
	else country1 = feasibleSubpopulation[place1] ;

	int place2 = std::rand() % (feasibleSubpopulation.size() + infeasibleSubpopulation.size()) ;
	if (place2 >= (int)feasibleSubpopulation.size()) country2 = infeasibleSubpopulation[place2 - feasibleSubpopulation.size()] ;
	else country2 = feasibleSubpopulation[place2] ;

	if (country1->biasedFitness < country2->biasedFitness) return country1 ;
	else return country2 ;		
}

Country * Population::getRandomCountry()
{
	Country * country1;

	updateBiasedFitnesses(feasibleSubpopulation);
	updateBiasedFitnesses(infeasibleSubpopulation);

	int place1 = std::rand() % (feasibleSubpopulation.size() + infeasibleSubpopulation.size()) ;
	if (place1 >= (int)feasibleSubpopulation.size()) country1 = infeasibleSubpopulation[place1 - feasibleSubpopulation.size()] ;
	else country1 = feasibleSubpopulation[place1] ;

	return country1;
}

Country * Population::getBestFeasible ()
{
	if (!feasibleSubpopulation.empty()) return feasibleSubpopulation[0] ;
	else return NULL ;
}

Country * Population::getBestInfeasible ()
{
	if (!infeasibleSubpopulation.empty()) return infeasibleSubpopulation[0] ;
	else return NULL ;
}

Country * Population::getBestFound()
{
	if (bestSolutionOverall.myCostSol.penalizedCost < 1.e29) return &bestSolutionOverall;
	else return NULL;
}

void Population::printState(int nbIter, int nbIterNoImprovement)
{
	std::printf("It %6d %6d | T(s) %.2f", nbIter, nbIterNoImprovement, (double)clock() / (double)CLOCKS_PER_SEC);

	if (getBestFeasible() != NULL) std::printf(" | Feas %zu %.2f %.2f", feasibleSubpopulation.size(), getBestFeasible()->myCostSol.penalizedCost, getAverageCost(feasibleSubpopulation));
	else std::printf(" | NO-FEASIBLE");

	if (getBestInfeasible() != NULL) std::printf(" | Inf %zu %.2f %.2f", infeasibleSubpopulation.size(), getBestInfeasible()->myCostSol.penalizedCost, getAverageCost(infeasibleSubpopulation));
	else std::printf(" | NO-INFEASIBLE");

	std::printf(" | Div %.2f %.2f", getDiversity(feasibleSubpopulation), getDiversity(infeasibleSubpopulation));
	std::printf(" | Feas %.2f %.2f", (double)std::count(listFeasibilityLoad.begin(), listFeasibilityLoad.end(), true) / (double)listFeasibilityLoad.size(), (double)std::count(listFeasibilityDuration.begin(), listFeasibilityDuration.end(), true) / (double)listFeasibilityDuration.size());
	std::printf(" | Pen %.2f %.2f", params->penaltyCapacity, params->penaltyDuration);
	std::cout << std::endl;
}

double Population::getDiversity(const SubPopulation & pop)
{
	double average = 0.;
	int size = std::min<int>(params->mu, pop.size()); // Only monitoring the "mu" better solutions to avoid too much noise in the measurements
	for (int i = 0; i < size; i++) average += pop[i]->averageBrokenPairsDistanceClosest(size);
	if (size > 0) return average / (double)size;
	else return -1.0;
}

double Population::getAverageCost(const SubPopulation & pop)
{
	double average = 0.;
	int size = std::min<int>(params->mu, pop.size()); // Only monitoring the "mu" better solutions to avoid too much noise in the measurements
	for (int i = 0; i < size; i++) average += pop[i]->myCostSol.penalizedCost;
	if (size > 0) return average / (double)size;
	else return -1.0;
}

// void Population::exportBKS(std::string fileName)
// {
// 	double readCost;
// 	std::vector<std::vector<int>> readSolution;
// 	std::cout << "----- CHECKING FOR POSSIBLE BKS UPDATE" << std::endl;
// 	bool readOK = Country::readCVRPLibFormat(fileName, readSolution, readCost);
// 	if (bestSolutionOverall.myCostSol.penalizedCost < 1.e29 && (!readOK || bestSolutionOverall.myCostSol.penalizedCost < readCost - MY_EPSILON))
// 	{
// 		std::cout << "----- NEW BKS: " << bestSolutionOverall.myCostSol.penalizedCost << " !!!" << std::endl;
// 		bestSolutionOverall.exportCVRPLibFormat(fileName);
// 	}
// }

void Population::exportSearchProgress(std::string fileName, std::string instanceName, int seedRNG, int prevtime)
{
    std::ofstream myfile(fileName);
    for (std::pair<clock_t, double> state : searchProgress)
        // myfile << instanceName << ";" << seedRNG << ";" << state.second << ";" << (double)state.first / (double)CLOCKS_PER_SEC - (double)prevtime << std::endl;
        myfile << instanceName << ";" << seedRNG << ";" << std::fixed << std::setprecision(6) << state.second << ";" << std::fixed << std::setprecision(3) << (double)state.first / (double)CLOCKS_PER_SEC - (double)prevtime << std::endl;
}

// void Population::exportSearchProgress(std::string fileName, std::string instanceName, int seedRNG, int prevtime)
// {
// 	std::ofstream myfile(fileName);
// 	// std::ofstream myfile2("../PerformanceProgress/test1.csv");

// 	double lastBestValue = searchProgress.front().second;
// 	double solTime = 0;
// 	double lowerPercent = 0;
// 	double upperPercent = 0;

// 	for (std::pair<clock_t, double> state : searchProgress)
// 	{
// 		solTime = (double)state.first / (double)CLOCKS_PER_SEC - (double)prevtime;
// 		myfile << instanceName << ";" << seedRNG << ";" << state.second << ";" << solTime << std::endl;
// 		lowerPercent = (perfStages[perfPointer] * (double)params->instanceTimelimit)/100;
// 		upperPercent = (perfStages[perfPointer + 1] * (double)params->instanceTimelimit)/100;
// 		if (solTime > lowerPercent && solTime < upperPercent)
// 		{
// 			performanceProgess.push_back({perfStages[perfPointer], lastBestValue});
// 			perfPointer ++;
// 		}
// 		else if (solTime > upperPercent)
// 		{
// 			while (solTime > (perfStages[perfPointer] * (double)params->instanceTimelimit)/100)
// 			{
// 				performanceProgess.push_back({perfStages[perfPointer], lastBestValue});
// 				perfPointer ++;
// 			}
// 		}
// 		lastBestValue = state.second;
// 	}
// 	for (int i = perfPointer ; i < 10 ; i++)
// 	{
// 		performanceProgess.push_back({perfStages[i], lastBestValue});
// 	}
// }

// void Population::exportPerformanceProgress(std::string fileName)
// {
// 	std::ofstream myfile(fileName);
// 	double seconds = 0;
// 	for (std::pair<double, double> state : performanceProgess)
// 	{
// 		seconds = state.first * (double)params->instanceTimelimit / 100;
// 		myfile << state.first << ";" << state.second << ";" << seconds << std::endl;
// 	}
		
// }

Population::Population(Params * params, Split * split, LocalSearch * localSearch) : params(params), split(split), localSearch(localSearch)
{
	listFeasibilityLoad = std::list<bool>(100, true);
	listFeasibilityDuration = std::list<bool>(100, true);
	// generatePopulation();
}

Population::Population()
{
	listFeasibilityLoad = std::list<bool>(100, true);
	listFeasibilityDuration = std::list<bool>(100, true);
}

Population::~Population()
{
	for (int i = 0; i < (int)feasibleSubpopulation.size(); i++) delete feasibleSubpopulation[i];
	for (int i = 0; i < (int)infeasibleSubpopulation.size(); i++) delete infeasibleSubpopulation[i];
}