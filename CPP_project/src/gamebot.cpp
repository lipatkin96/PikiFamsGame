#include "gamebot.h"

#include <ctime>
#include <limits.h>

namespace PikiFamsGameBot {

	//TODO: replace WorldSet on any_range (only reading the world container)
	void combineTheBestStep(const WorldSet & world, StepStructure & stepStructure) 
	{
		int minLength = INT_MAX;		//1 - it is our valid minimum (not 0)

		stepStructure.resize(1u);

		//1. Sort out the one-set sets

		stepStructure[0].size = stepLength;
		for (WorldSetIterator it = world.cbegin(); it != world.cend(); it++) {
			if (it->plenty.size() > stepLength) {	//size == stepLength - is useless because we will know nothing new about that set
				PossibleResultInterval posResSet = calculatePossibleResultSet(*it, stepLength);
				//signed int for unpredictable behavior
				int combinationCount = posResSet.right() - posResSet.left();	
						//My criterion for selection best step
				//TODO: create the criterion-lambda function
				if (combinationCount > 0 && combinationCount < minLength) {
					stepStructure[0].baseIt = it;
					minLength = combinationCount;
				}
			}
		}

		//2. Sort out the two-set sets

		if (!world.empty()) {
			bool flagFindNew = false;
			stepStructure.resize(2u);

			for (WorldSetIterator iIt = world.cbegin(); (iIt + 1) != world.cend(); iIt++) {
				for (WorldSetIterator jIt = iIt + 1; jIt != world.cend(); jIt++) {
					//We have two sets: *iIt, *jIt

					//else - is useless because we will know nothing new about these sets
					if (iIt->plenty.size() + jIt->plenty.size() > stepLength) {

						//Sum of subset length = stepLength

						//Variants of subset length = { <1, 3>; <2, 2>; <3, 1> } ~ <iIt, jIt>

						auto updateStepStructure = 
							[&stepStructure, &minLength, &flagFindNew](WorldSetIterator fisrtIt,
								uint32_t firstLen, WorldSetIterator secondIt, uint32_t secondLen) 
						{
							if (fisrtIt->plenty.size() >= firstLen && secondIt->plenty.size() >= secondLen)
							{
								PossibleResultInterval posResSet =
									calculatePossibleResultSet(*fisrtIt, firstLen, *secondIt, secondLen);
								int combinationCount = posResSet.right() - posResSet.left();
								if (combinationCount > 0 && combinationCount < minLength) {
									stepStructure[0] = { fisrtIt, firstLen };
									stepStructure[1] = { secondIt, secondLen };
									minLength = combinationCount;
									flagFindNew = true;
								}
							}
						};

						//The order of execution is important because for my criterion
						//no mean: {5, 6, Zero, Zero } or {5, Zero, Zero, Zero} from the {5, 6, 7, 8} = 1
						// but first one is better. I know.

						// <2, 2>
						updateStepStructure(iIt, 2, jIt, 2);
						// <1, 3>
						updateStepStructure(iIt, 1, jIt, 3);
						// <3, 1>
						updateStepStructure(iIt, 3, jIt, 1);
					}
				}
			}
			if (!flagFindNew)
				stepStructure.resize(1u);
		}

		//Function Output: stepStructure
	}

	//TODO: may updateWorld and calculateResidue must be together
	void updateWorld(WorldSet & world, const GameSet & step, const StepStructure & stepStructure)	
	{
		GameSet residue;		//why residue it is only one set?
		StepStructure stepRelations = stepStructure;	//it is relation between residue and lost stepStructure

		//-------------Calculate residue-------------//

		//Works only at beginning two steps
		if (stepStructure.size() == 1) {	//It means that step is subset of a world's set
											//1. Exclude from superset the step to get residue
			const DigitSet & plenty = stepStructure[0].baseIt->plenty;
			DigitSet temp;
			std::set_difference(plenty.cbegin(), plenty.cend(),
				step.plenty.cbegin(), step.plenty.cend(),
				std::inserter(temp, temp.begin()));

			residue.plenty = temp;
			residue.value = stepStructure[0].baseIt->value - step.value;
		}
		else {
			//1. exclude from step sets that we use for combining this step 
			//and belong to world set (i.e. are the entire set)

			residue = step;
			for (uint32_t i = 0; i < stepStructure.size(); i++) {
				const DigitSet & plenty = stepStructure[i].baseIt->plenty;
				if (plenty.size() == stepStructure[i].size) {	//if set that we use has size the same as its superset
					DigitSet temp;
					std::set_difference(residue.plenty.cbegin(), residue.plenty.cend(),
						plenty.cbegin(), plenty.cend(), std::inserter(temp, temp.begin()));
					residue.plenty = temp;
					residue.value -= stepStructure[i].baseIt->value;

					stepRelations.erase(stepRelations.begin() + i);
				}
			}
		}

		//------------------Updating the world-----------------//

		if (stepStructure.size() == 1)	//It means that step is subset of a world's set
		{
			WorldSet::iterator it = world.begin();
			std::advance(it, 
				std::distance<WorldSet::const_iterator>(world.cbegin(), stepStructure[0].baseIt));
			*it = residue;
			world.push_back(step);	//! It must be executed after using iterator information
		}

		//TODO: you don't finish work!!!! (Update the world set)

		if (residue.plenty.size() == residue.value || 0 == residue.value) {
			DigitSet::const_iterator it;
			for (it = residue.plenty.cbegin(); it != residue.plenty.cend(); it++)
				world.emplace_back(DigitSet({ *it }),
					(residue.value > 0u));	//every set has value 1 (if set.size == value),
											//or 0 (if set.size == 0)
			//not finish
		}
		else {
			//world.push_back(residue);	//?????
		}
	}





	//-------------------------------------------------------------------//
	//-------------------------Central Unit------------------------------//
	//-------------------------------------------------------------------//



	SolvingInfo solveTheGame(GameCreator game) 
	{
		std::srand(static_cast<uint32_t>(time(0)));

		//Start

		GameSet digits(DigitSet({ 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 }), stepLength);

		//TODO: read about reserve and clear: how you can use vector array without reallocation
		//TODO: think about what be better: vector or list. You need to sort out the array and delete items from it
		WorldSet world({ digits });		//world is a set of current GameSets
		StepStructure stepStructure;
		stepStructure.reserve(stepLength);

		GameSet step;
		DigitArray appropriateStep;
		do {
			stepStructure.clear();
			step.plenty.clear();

			//--------------------------Collect the necessary information----------------------------//

			//TODO: necessary test it
			//Input: world
			combineTheBestStep(world, stepStructure);
			//Output: stepStructure

			//--------------------------Building the step--------------------------//

			//Not interesting to test
			for (uint32_t i = 0; i < stepStructure.size(); i++) {
				DigitSet plenty = stepStructure[i].baseIt->plenty;
				DigitSet::const_iterator it;
				for (uint32_t j = 0; j < stepStructure[i].size; j++) {
					it = plenty.begin();
					std::advance(it, rand() % plenty.size());
					step.plenty.insert(*it);
					plenty.erase(it);
				}
			}

			//--------------------------Try to guess--------------------------//

			std::copy(step.plenty.cbegin(), step.plenty.cend(), appropriateStep.begin());
			GameStepInfo info = game.guess(appropriateStep);
			step.value = info.fams + info.piki;
			if (step.value >= stepLength)
				break;

			//Output

			std::cout << "Guess number: ";
			std::copy(appropriateStep.begin(), appropriateStep.end(),
				std::ostream_iterator<DigitType>(std::cout, " "));
			std::cout << ", " << step.value << std::endl;

			//--------------------------Analyzing the step--------------------------//

			//TODO: Test it
			//1. Calculate the residue
			//GameSet residue = calculateResidue(world, step, stepStructure);

			//TODO: Test it
			//2. Analysing the residue or update the world
			updateWorld(world, step, stepStructure);

		} while (0 == 1);

		//Wherever end of cycle
		
		return { step.plenty, game.getStepCount() };
	}

}
