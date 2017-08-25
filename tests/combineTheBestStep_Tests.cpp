#include "Test_hd.h"

#include <unordered_set>
#include <set>
//#include <set_mu>
#include <iostream>

#include "..\src\gamebot.h"

using PikiFamsGameBot::GameSet;
using PikiFamsGameBot::StepStructure;
using PikiFamsGameBot::WorldSet;
using PikiFamsGameBot::WorldSetIterator;

using std::unordered_set;
using std::set;
using std::multiset;

using std::cout;
using std::endl;

using PikiFamsGameBot::combineTheBestStep;

//--------------------Combine The Best Step Tests--------------------//

//These tests are separate on two groups : first group must be complete in the whole evolution of Game.
//Second one depends on only current algorithms for combining best step. It can be omit with new version.

TEST(TestCombineTheBestStep, first) {
	StepStructure stepStruct;
	//WorldSet - is vector
	WorldSet world({ 
		GameSet({ 1, 2, 3, 4 }, 2),
		GameSet({ 5, 6, 7, 8 }, 1), 
		GameSet({ 0, 9 }, 1)
	});
	combineTheBestStep(world, stepStruct);
	EXPECT_TRUE(stepStruct.size() > 1 
		&& (set<WorldSetIterator>({ (stepStruct[0].baseIt), (stepStruct[1].baseIt) })
			== set<WorldSetIterator>({ world.begin() + 1, world.begin() + 2 }))
		&& (multiset<uint32_t>({2, 2}) == multiset<uint32_t>({ stepStruct[0].size, stepStruct[1].size }))
	) //<< "value: " << stepStruct[1].baseIt->value << " " << stepStruct[0].baseIt->value
		//<< " subSetLength: " << stepStruct[1].size << " " << stepStruct[0].size
		;
}


