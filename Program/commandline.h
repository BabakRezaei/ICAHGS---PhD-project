#ifndef COMMAND_LINE_H
#define COMMAND_LINE_H

#include <iostream>
#include <string>
#include <climits>
#include <sstream>

class CommandLine
{
public:
	int timeLimit         = INT_MAX;	// CPU time limit until termination in seconds. Default value: infinity
	int seed		      = 0;			// Random seed. Default value: 0
	int nbVeh		      = INT_MAX;	// Number of vehicles. Default value: infinity
	int NumOfDecades      = 2500;       // Number of decades
	int InitialCountries  = 100;        // Initial countries
	int CompetitionPeriod = 10;			// period between Imperialistic competitions (in decade)
	int bks               = INT_MAX;    // to compare with BKS when running for new BKS
	int nbIter	          = 20000;		// Number of iterations without improvement until termination. Default value: 20,000 iterations
	std::string pathInstance;		// Instance path
	std::string pathSolution;		// Solution path
	std::string pathPerformance;	// Performance path
	std::string pathBKS = "";		// BKS path
	std::string instanceName = "";  // The name of VRP instance
	std::vector < std::string > instances;
	std::vector < int > seeds;
	std::vector < int > initialImp;  // number of initial imperialists
	std::vector < double > RevolRate; // revolution Rate

	// Reads the line of command and extracts possible options
	CommandLine(int argc, char* argv[])
	{
		pathInstance = "../Instances/CVRP/";
		pathSolution = "../GOLDEN/Sol-";
		pathPerformance = "../PerformanceProgress/Perf-";
		// instanceName = std::string(argv[3]);

		for (int i = 1; i < argc; i += 2)
		{
			if (std::string(argv[i]) == "-i")
				instances.push_back(std::string(argv[i+1]));
			else if (std::string(argv[i]) == "-s")
				seeds.push_back(atoi(argv[i+1]));
			else if (std::string(argv[i]) == "-cmp")
				CompetitionPeriod = atoi(argv[i+1]);
			else if (std::string(argv[i]) == "-t")
				timeLimit = atoi(argv[i+1]);
			else if (std::string(argv[i]) == "-bks")
				bks = atoi(argv[i+1]);
			else if (std::string(argv[i]) == "-it")
				nbIter = atoi(argv[i+1]);
			else if (std::string(argv[i]) == "-p")
				InitialCountries = atoi(argv[i+1]);
			else if (std::string(argv[i]) == "-ii")
				initialImp.push_back(atoi(argv[i+1]));
			else if (std::string(argv[i]) == "-rr")
				RevolRate.push_back(atof(argv[i+1]));
			else if (std::string(argv[i]) == "-veh")
				nbVeh = atoi(argv[i+1]);
			else
			{
				std::cout << "----- ARGUMENT NOT RECOGNIZED: " << std::string(argv[i]) << std::endl;
				display_help(); throw std::string("Incorrect line of command");
			}
		}
		if ((timeLimit != INT_MAX || bks != INT_MAX) && (int)instances.size() > 1)
			throw std::string("Incorrect line of command");

		if ((int)instances.size() == 0) throw std::string("You must input instance name using -i");
		if ((int)seeds.size() == 0) seeds.push_back(1);
		if ((int)initialImp.size() == 0) initialImp.push_back(3);
		if ((int)RevolRate.size() == 0) RevolRate.push_back(0.5);
	}

	// Printing information about how to use the code
	void display_help()
	{
		std::cout << std::endl;
		std::cout << "------------------------------------------------- HICA-CVRP algorithm (2021) --------------------------------------------------" << std::endl;
		std::cout << "Call with: ./icavrp instancePath solPath [-dc nbDecade] [-t myCPUtime] [-bks bksPath] [-seed mySeed] [-veh nbVehicles]         " << std::endl;
		std::cout << "[-dc nbDecade] sets a maximum number of decades to terminate algorithm. Defaults to 200                                        " << std::endl;
		std::cout << "[-t myCPUtime] sets a time limit in seconds. If this parameter is set the code will be run iteratively until the time limit    " << std::endl;
		std::cout << "[-bks bksPath] sets an optional path to a BKS. This file will be overwritten in case of improvement                            " << std::endl;
		std::cout << "[-seed mySeed] sets a fixed seed. Defaults to 0                                                                                " << std::endl;
		std::cout << "[-veh nbVehicles] sets a prescribed fleet size. Otherwise a reasonable UB on the the fleet size is calculated                  " << std::endl;
		std::cout << "-------------------------------------------------------------------------------------------------------------------------------" << std::endl;
		std::cout << std::endl;
	};
};
#endif
