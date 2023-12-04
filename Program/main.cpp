#include "ICA.h"
#include "commandline.h"
#include "LocalSearch.h"
#include "Split.h"

using namespace std;

int main(int argc, char *argv[])
{
	try
	{
		int timelimit = 0;
		int prevtimelimit = 0;
		int bks = 0;

		// Reading the arguments of the program
		CommandLine commandline(argc, argv);
		if (commandline.bks != INT_MAX) bks = commandline.bks;
    
		std::cout << "----- Algorithm will be run for following instances: " << std::endl;
		for (string instanceName : commandline.instances)
			std::cout << "Instance name: " << instanceName << std::endl;
    
		std::cout << "----- Calculations will be done for following seeds: " << std::endl;
		for (int seed : commandline.seeds)
			std::cout << "Seed number: " << std::to_string(seed) << std::endl;

		for (string instanceName : commandline.instances)
		{
			// std::string pathInstance = "../Instances/CVRP/" + instanceName + ".vrp";
			std::string pathInstance = commandline.pathInstance + instanceName + ".vrp";

			for (int seed : commandline.seeds)
			{
				for (int initimp : commandline.initialImp)
				{
					for (double RevolRate : commandline.RevolRate)
					{
						// Reading the data file and initializing some data structures
						std::cout << "----- READING DATA SET: " << pathInstance << std::endl;
						Params params(pathInstance, commandline.nbVeh, seed, commandline.InitialCountries, initimp, RevolRate);

						// Creating the Split and local search structures
						Split split(&params);
						LocalSearch localSearch(&params);

						std::cout << "----- INSTANCE LOADED WITH " << params.nbClients << " CLIENTS AND " << params.nbVehicles << " VEHICLES" << std::endl;
                    
						prevtimelimit = timelimit;
						if (commandline.timeLimit != INT_MAX) timelimit = timelimit + commandline.timeLimit;
						else if (commandline.timeLimit == INT_MAX && bks == 0) timelimit = timelimit + std::round(params.nbClients * 2.4 * 2.4 / 1.6);
						else if (commandline.timeLimit == INT_MAX && bks != 0) timelimit = INT_MAX;

						// Imperialist Competition Algorithm (ICA)
						ICA solver(&params, &split, &localSearch);
						solver.run(commandline.NumOfDecades, timelimit, commandline.CompetitionPeriod, seed, bks, commandline.nbIter, prevtimelimit);
						std::cout << "----- ICA ALGORITHM FINISHED, TIME SPENT: " << (double)clock()/(double)CLOCKS_PER_SEC - (double)prevtimelimit << std::endl;

						// Exporting the best solution
						Country * bestSolution = NULL;
						bestSolution = solver.Empires[0]->population->getBestFound();
						double bestclk = solver.Empires[0]->population->bestfoundclock;
						int bestindex = 0;
						for(int i = 1; i < (int)solver.Empires.size(); i++)
						{
							double cost = solver.Empires[i]->population->getBestFound()->myCostSol.penalizedCost;
							double clk = solver.Empires[i]->population->bestfoundclock;
							if (cost < bestSolution->myCostSol.penalizedCost)
							{
								bestSolution = solver.Empires[i]->population->getBestFound();
								bestclk = solver.Empires[i]->population->bestfoundclock;
								bestindex = i;
							}	
							else if (cost == bestSolution->myCostSol.penalizedCost)
							{
								if (clk < bestclk)
								{
									bestSolution = solver.Empires[i]->population->getBestFound();
									bestclk = solver.Empires[i]->population->bestfoundclock;
									bestindex = i;
								}
							}
						}

						std::string pathSolution = commandline.pathSolution + instanceName + "-Seed-" + std::to_string(seed)
												+ "-Cntry-" + std::to_string(params.NumOfCountries)
												+ "-Emp-" + std::to_string(params.NumOfInitialImperialists)
												+ "-Cmp-" + std::to_string(commandline.CompetitionPeriod)
												+ "-itr-" + std::to_string(commandline.nbIter)
												+ "-RR-" + std::to_string(params.RevolutionRate);
						bestSolution->exportCVRPLibFormat(pathSolution + ".txt", prevtimelimit, bestclk);
						solver.Empires[bestindex]->population->exportSearchProgress(pathSolution + ".csv", pathInstance, seed, prevtimelimit);
					}
				}
			}
		}
	}
	catch (const string& e) { std::cout << "EXCEPTION | " << e << std::endl; }
	catch (const std::exception& e) { std::cout << "EXCEPTION | " << e.what() << std::endl; }
	return 0;
}
