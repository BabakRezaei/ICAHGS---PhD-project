#ifndef ICA_H
#define ICA_H

#include "Country.h"
#include "Split.h"
#include "LocalSearch.h"
#include "Params.h"
#include "Population.h"

struct Empire
{

public:

    Country * Imperialist;
	double cost;
	double zeta;
	int colonySize;
	Population * population;
	int lastResetDecade = INT_MAX;
	int nextRestartDecade = 0;
	int managePenaltyCounter = 0;
	int NumOfResets = 0;
	double LastResetValue = 0.;
	bool AssimType = false;  // false=assimilation with colonies, true=assimilation with best solution/imperialist

	void calculateCost()
	{	
		double empCost = 0;
		int counter = 0;
		for (int i = 1 ; (i < (int)population->feasibleSubpopulation.size() && counter < colonySize) ; i++)
		{
			empCost += population->feasibleSubpopulation[i]->myCostSol.penalizedCost;
			counter++;
		}
		for (int j = 1 ; (j < (int)population->infeasibleSubpopulation.size() && counter < colonySize) ; j++)
		{
			empCost += population->infeasibleSubpopulation[j]->myCostSol.penalizedCost;
			counter++;
		}
		cost = population->feasibleSubpopulation[0]->myCostSol.penalizedCost + zeta * (empCost / (double)colonySize);
			
	}

	Empire(double pzeta) {cost = 1.e30; zeta = pzeta;};
	~Empire()
	{
		delete Imperialist;
	};
};

class ICA
{
private:
	Params * params;				// Problem parameters
	Split * split;					// Split algorithm
	Country * country;		        // Country
	LocalSearch * localSearch;		// Local Search structure

	// OX Crossover
	void mycrossoverOX(Country * result, const Country * country1, const Country * country2);

	// Generate initial countries
	void CreateCountries();

	// Create Empires from initial countries
	void CreateEmpires();

	// Assimilation the colonies of each Empire
	void AssimilateColonies();

	// simulating the Revolution for each Empire
	void RevolveColonies();

	// Posses empires. run for each Empire
	void PossesEmpire();

	// Unite similar empires
	void UniteSimilarEmpires(double diff);

	// Imperialistic competition
	void ImperialisticCompetition();

	// Mix Assimilation and Revolution
	void AssimRevolve(bool changeAssimilationType, int decade, int nbIter);

	void ParallelAssimRevolve(bool changeAssimilationType, int decade);

	void RestartEmpires(Empire * empire);

	void CreateNewCountries(Empire * empire);

	void NewCreateNewCountries(Empire * empire);

	Country * generateHQImperialist();

	Country * generateNewImperialist();

	

public:
	std::vector < Empire* > Empires;
	Country * bestSolutionOverall;

    // Running the ICA algorithm until maxIterNonProd consecutive iterations
    void run(int Decades, int timeLimit, int CompetitionPeriod, int seed, int bks, int nbIter, int prevtime);

	// Accesses the best found solution at all time
	Country * getBestFound();

	// Constructor
	ICA(Params * params, Split * split, LocalSearch * localSearch);

	// Destructor
	~ICA(void);
};

#endif