/*********************************************************************
* Software License Agreement (BSD License)
* 
*  Copyright (c) 2008, Willow Garage, Inc.
*  All rights reserved.
* 
*  Redistribution and use in source and binary forms, with or without
*  modification, are permitted provided that the following conditions
*  are met:
* 
*   * Redistributions of source code must retain the above copyright
*     notice, this list of conditions and the following disclaimer.
*   * Redistributions in binary form must reproduce the above
*     copyright notice, this list of conditions and the following
*     disclaimer in the documentation and/or other materials provided
*     with the distribution.
*   * Neither the name of the Willow Garage nor the names of its
*     contributors may be used to endorse or promote products derived
*     from this software without specific prior written permission.
* 
*  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
*  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
*  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
*  FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
*  COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
*  INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
*  BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
*  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
*  CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
*  LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
*  ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
*  POSSIBILITY OF SUCH DAMAGE.
*********************************************************************/

/* \author Ioan Sucan */

#include "ompl/extension/samplingbased/kinematic/extension/sbl/LRSBL.h"

bool ompl::LRSBL::solve(double solveTime)
{
    SpaceInformationKinematic_t                          si = dynamic_cast<SpaceInformationKinematic_t>(m_si);
    SpaceInformationKinematic::GoalRegionKinematic_t goal_r = dynamic_cast<SpaceInformationKinematic::GoalRegionKinematic_t>(si->getGoal());
    SpaceInformationKinematic::GoalStateKinematic_t  goal_s = dynamic_cast<SpaceInformationKinematic::GoalStateKinematic_t>(si->getGoal());
    unsigned int                                        dim = si->getStateDimension();
    
    if (goal_s)
	return SBL::solve(solveTime);
    
    if (!goal_r)
    {
	m_msg.error("Unknown type of goal (or goal undefined)");
	return false;
    }
    
    time_utils::Time endTime = time_utils::Time::now() + time_utils::Duration(solveTime);
    
    /* memory for temporary goal */
    SpaceInformationKinematic::GoalStateKinematic_t stateGoal = new SpaceInformationKinematic::GoalStateKinematic(si);
    stateGoal->state = new SpaceInformationKinematic::StateKinematic(dim);
    
    bool solved = false;
    
    while (!solved)
    {
	m_lazyRRT.clear();
	if (m_lazyRRT.solve((endTime - time_utils::Time::now()).to_double()))
	{
	    SpaceInformationKinematic::PathKinematic_t foundPath = static_cast<SpaceInformationKinematic::PathKinematic_t>(goal_r->getSolutionPath());
	    if (foundPath && foundPath->states.size() == 1)
	    {
		/* change goal to a state one */
		si->forgetGoal();
		si->setGoal(stateGoal);
		si->copyState(stateGoal->state, foundPath->states[0]);
		
		/* run SBL on the new goal */
		solved = SBL::solve((endTime - time_utils::Time::now()).to_double());
		
		/* restore user-set goal */
		si->forgetGoal();
		si->setGoal(goal_r);
		
		/* copy solution to actual goal instance */
		if (solved)
		{
		    goal_r->setSolutionPath(stateGoal->getSolutionPath());
		    stateGoal->forgetSolutionPath();
		}
	    }
	}
    }
    
    delete stateGoal;
    return solved;
}
