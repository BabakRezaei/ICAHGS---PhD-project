#include "Country.h"
#include "iomanip"

void Country::evaluateCompleteCost()
{
	myCostSol = CostSol();
	for (int r = 0; r < params->nbVehicles; r++)
	{
		if (!chromR[r].empty())
		{
			double distance = params->timeCost[0][chromR[r][0]];
			double load = params->cli[chromR[r][0]].demand;
			double service = params->cli[chromR[r][0]].serviceDuration;
			predecessors[chromR[r][0]] = 0;
			for (int i = 1; i < (int)chromR[r].size(); i++)
			{
				distance += params->timeCost[chromR[r][i-1]][chromR[r][i]];
				load += params->cli[chromR[r][i]].demand;
				service += params->cli[chromR[r][i]].serviceDuration;
				predecessors[chromR[r][i]] = chromR[r][i-1];
				successors[chromR[r][i-1]] = chromR[r][i];
			}
			successors[chromR[r][chromR[r].size()-1]] = 0;
			distance += params->timeCost[chromR[r][chromR[r].size()-1]][0];
			myCostSol.distance += distance;
			myCostSol.nbRoutes++;
			if (load > params->vehicleCapacity) myCostSol.capacityExcess += load - params->vehicleCapacity;
			if (distance + service > params->durationLimit) myCostSol.durationExcess += distance + service - params->durationLimit;
		}
	}

	myCostSol.penalizedCost = myCostSol.distance + myCostSol.capacityExcess*params->penaltyCapacity + myCostSol.durationExcess*params->penaltyDuration;
	isFeasible = (myCostSol.capacityExcess < MY_EPSILON && myCostSol.durationExcess < MY_EPSILON);
}

void Country::removeProximity(Country * cntry)
{
	auto it = cntrysPerProximity.begin();
	while (it->second != cntry) ++it;
	cntrysPerProximity.erase(it);
}

double Country::brokenPairsDistance(Country * cntry2)
{
	int differences = 0;
	for (int j = 1; j <= params->nbClients; j++)
	{
		if (successors[j] != cntry2->successors[j] && successors[j] != cntry2->predecessors[j]) differences++;
		if (predecessors[j] == 0 && cntry2->predecessors[j] != 0 && cntry2->successors[j] != 0) differences++;
	}
	return (double)differences/(double)params->nbClients;
}

double Country::averageBrokenPairsDistanceClosest(int nbClosest) 
{
	double result = 0 ;
	int maxSize = std::min<int>(nbClosest, cntrysPerProximity.size());
	auto it = cntrysPerProximity.begin();
	for (int i=0 ; i < maxSize; i++)
	{
		result += it->first ;
		++it ;
	}
	return result/(double)maxSize ;
}

void Country::exportCVRPLibFormat(std::string fileName, int timelimit, double bestfoundTime)
{
    std::cout << "----- WRITING SOLUTION WITH VALUE " << myCostSol.penalizedCost << " IN : " << fileName << std::endl;
    std::ofstream myfile(fileName);
    if (myfile.is_open())
    {
        for (int k = 0; k < params->nbVehicles; k++)
        {
            if (!chromR[k].empty())
            {
                myfile << "Route #" << k+1 << ":"; // Route IDs start at 1 in the file format
                for (int i : chromR[k]) myfile << " " << i;
                myfile << std::endl;
            }
        }
        myfile << "Cost " << std::fixed << std::setprecision(6) << myCostSol.penalizedCost << std::endl;
        myfile << "At time  " << std::fixed << std::setprecision(3) << (double)bestfoundTime / (double)CLOCKS_PER_SEC - (double)timelimit << std::endl;
        myfile << "Total Time " << (double)clock()/(double)CLOCKS_PER_SEC - (double)timelimit << std::endl;
        myfile << "Initial Population " << params->NumOfCountries << std::endl;
        myfile << "Initial Islands " << params->NumOfInitialImperialists << std::endl;
    }
    else std::cout << "----- IMPOSSIBLE TO OPEN: " << fileName << std::endl;
}

bool Country::readCVRPLibFormat(std::string fileName, std::vector<std::vector<int>> & readSolution, double & readCost)
{
	readSolution.clear();
	std::ifstream inputFile(fileName);
	if (inputFile.is_open())
	{
		std::string inputString;
		inputFile >> inputString;
		// Loops as long as the first line keyword is "Route"
		for (int r = 0; inputString == "Route" ; r++) 
		{
			readSolution.push_back(std::vector<int>());
			inputFile >> inputString;
			getline(inputFile, inputString);
			std::stringstream ss(inputString);
			int inputCustomer;
			while (ss >> inputCustomer) // Loops as long as there is an integer to read
				readSolution[r].push_back(inputCustomer);
			inputFile >> inputString;
		}
		if (inputString == "Cost")
		{
			inputFile >> readCost;
			return true;
		}
		else std::cout << "----- UNEXPECTED WORD IN SOLUTION FORMAT: " << inputString << std::endl;
	}
	else std::cout << "----- IMPOSSIBLE TO OPEN: " << fileName << std::endl;
	return false;
}

Country::Country(Params * params) : params(params)
{
	successors = std::vector <int>(params->nbClients + 1);
	predecessors = std::vector <int>(params->nbClients + 1);
	chromR = std::vector < std::vector <int> >(params->nbVehicles);
	chromT = std::vector <int>(params->nbClients);
	for (int i = 0; i < params->nbClients; i++) chromT[i] = i + 1;
	std::random_shuffle(chromT.begin(), chromT.end());
}

Country::Country()
{
	myCostSol.penalizedCost = 1.e30;
}
