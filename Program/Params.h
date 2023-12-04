#ifndef PARAMS_H
#define PARAMS_H

#include "CircleSector.h"
#include <string>
#include <vector>
#include <list>
#include <set>
#include <fstream>
#include <iostream>
#include <sstream>
#include <cmath>
#include <time.h>
#include <climits>
#include <algorithm>
#include <unordered_set>
#define MY_EPSILON 0.00001 // Precision parameter, used to avoid numerical instabilities
#define PI 3.14159265359

struct Client
{
	int custNum;			// Index of the customer	
	double coordX;			// Coordinate X
	double coordY;			// Coordinate Y
	double serviceDuration; // Service duration
	double demand;			// Demand
	int polarAngle;			// Polar angle of the client around the depot, measured in degrees and truncated for convenience
};

class Params
{
public:

	/* PARAMETERS OF THE ICA ALGORITHM */
	int nbGranular				 = 20;	 // Granular search parameter, limits the number of moves in the RI local search
	int nbClose					 = 5;	 // Number of closest solutions/countries considered when calculating diversity contribution
	double targetFeasible   	 = 0.2;	 // Reference proportion for the number of feasible countries, used for the adaptation of the penalty parameters
	int NumOfCountries      	 = 100;  // Number of initial countries
	// int NumOfDecades	         = 2500;  // Number of Decades to terminate ICA algorithm. Defualt value: 200 decades
	int NumOfInitialImperialists = 3;   // Number of initial empires
	double RevolutionRate        = 0.5;  // Revolution is the process in which the socio-political characteristics of a country change suddenly
	double Zeta  				 = 0.3;  // Total Cost of Empire = Cost of Imperialist + Zeta * mean(Cost of All Colonies)
	double DampRatio 			 = 0.95;
	int NumOfAllColonies; 

	int mu					= 25;		// Minimum population size
	int lambda				= 40;		// Number of solutions created before reaching the maximum population size (i.e., generation size)
	int nbElite				= 4;		// Number of elite individuals (reduced in HGS-2020)
	
	/* ADAPTIVE PENALTY COEFFICIENTS */
	double penaltyCapacity;				// Penalty for one unit of capacity excess (adapted through the search)
	double penaltyDuration;				// Penalty for one unit of duration excess (adapted through the search)

	/* DATA OF THE PROBLEM INSTANCE */			
	bool isRoundingInteger ;								// Distance calculation convention
	bool isDurationConstraint ;								// Indicates if the problem includes duration constraints
	int nbClients ;											// Number of clients (excluding the depot)
	int nbVehicles ;										// Number of vehicles
	double durationLimit;									// Route duration limit
	double vehicleCapacity;									// Capacity limit
	double totalDemand ;									// Total demand required by the clients
	double maxDemand;										// Maximum demand of a client
	double maxDist;											// Maximum distance between two clients
	std::vector < Client > cli ;							// Vector containing information on each client
	std::vector < std::vector < double > > timeCost ;		// Distance matrix
	std::vector < std::vector < int > > correlatedVertices;	// Neighborhood restrictions: For each client, list of nearby customers
	int instanceTimelimit;

	// Initialization from a given data set
	Params(std::string pathToInstance, int nbVeh, int seedRNG, int InitCntrys, int NumImps, double RevolRate);
};
#endif

