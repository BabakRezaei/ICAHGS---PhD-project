#ifndef COUNTRY_H
#define COUNTRY_H

#include "Params.h"

class Country;

struct CostSol
{
	double penalizedCost;		// Penalized cost of the solution
	int nbRoutes;				// Number of routes
	double distance;			// Total Distance
	double capacityExcess;		// Sum of excess load in all routes
	double durationExcess;		// Sum of excess duration in all routes
	CostSol() { penalizedCost = 0.; nbRoutes = 0; distance = 0.; capacityExcess = 0.; durationExcess = 0.; }
};

class Country
{
public:

  Params * params ;															// Problem parameters
  CostSol myCostSol;														// Solution cost parameters
  std::vector < int > chromT ;												// Giant tour representing the country
  std::vector < std::vector <int> > chromR ;								// For each vehicle, the associated sequence of deliveries (complete solution)
  std::vector < int > successors ;											// For each node, the successor in the solution (can be the depot 0)
  std::vector < int > predecessors ;										// For each node, the predecessor in the solution (can be the depot 0)
  std::multiset < std::pair < double, Country* > > cntrysPerProximity ;	// The other countrys in the population, ordered by increasing proximity (the set container follows a natural ordering based on the first value of the pair)
  bool isFeasible;															// Feasibility status of the country
  double biasedFitness;														// Biased fitness of the solution

  // Measuring cost of a solution from the information of chromR
  void evaluateCompleteCost();

  // Removing an country in the structure of proximity
  void removeProximity(Country * cntry);

  // Distance measure with another country
  double brokenPairsDistance(Country * cntry2);

  // Returns the average distance of this country with the nbClosest countrys
  double averageBrokenPairsDistanceClosest(int nbClosest) ;

  // Exports a solution in CVRPLib format (adds a final line with the computational time)
  void exportCVRPLibFormat(std::string fileName, int timelimit, double bestfoundTime);

  // Reads a solution in CVRPLib format, returns TRUE if the process worked, or FALSE if the file does not exist or is not readable
  static bool readCVRPLibFormat(std::string fileName, std::vector<std::vector<int>> & readSolution, double & readCost);

  // Constructor: random country
  Country(Params * params);

  // Constructor: empty country
  Country();
};
#endif
