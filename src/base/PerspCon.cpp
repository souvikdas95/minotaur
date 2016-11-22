//
//    MINOTAUR -- It's only 1/2 bull
//
//    (C)opyright 2009 - 2014 The MINOTAUR Team.
//

/**
 * \file PerspCon.cpp
 * \Define base class that determines separability perspective constraints.
 * \author Meenarli Sharma, IIT Bombay, India
 */

#include <iostream>
using std::endl;
using std::flush;

#include "PerspCon.h"
#include "Function.h"
#include "LinearFunction.h"
#include "QuadraticFunction.h"
#include "NonlinearFunction.h"
#include "Logger.h"

# define SPEW 1

using namespace Minotaur;

/*********************************************************************************/
//Functionality for identifying structure f(x) <=b or f(x,z) <=b and  lz<=x<=uz, z is 
//binary variable and x are continuous variables
/*********************************************************************************/

///Default constructor
  PerspCon::PerspCon()
:env_(EnvPtr()),p_(ProblemPtr()), cList_(0), binVar_(0), lbc_(0), ubc_(0), l_(0), u_(0)
{
  logger_=(LoggerPtr) new Logger(LogDebug2);
}

///Construtor for original problem
  PerspCon::PerspCon(ProblemPtr p, EnvPtr env)
: env_(env),p_(p), cList_(0), binVar_(0), lbc_(0), ubc_(0), l_(0), u_(0)
{
  /*perspConsInfo_=(InfoVecPtr) new ConsInfo();*/
  //stats_= new PerspConStats();
  //stats_->totalcons = 0;
  //stats_->totalpersp = 0;
  // Debug preparation.

  // Print number of perspective amenable constraints from handler in log
  // messages
}

/// Findng contraints suitable for perspective reformulation
//from the original problem
void PerspCon::generateList()
{
  // Pointer to current constraint being checked.
  ConstConstraintPtr cons;
  // Shows if the constraint is a perspective constraint.
  bool ispersp =  false;
  // Iterate through each constraint.
  for (ConstraintConstIterator it= p_->consBegin(); it!= p_->consEnd(); ++it) {
    cons = *it;
    //cInfo_ = new ConsInfo;
    // Binary variable 
    VariablePtr binvar; 
    // If binary variable is included 
    // Check if f(0) <= b. 
    //if (cons->getUb() >= 0) {
      //ispersp = evalConstraint(cons, binvar);
    //} else {
      //continue;    
    //}
    ispersp = evalConstraint(cons, binvar);
    if (ispersp) {
      // If it is a perpsective constraint, then add it to vector.
      //cInfo_->prCons(cons);
      //cInfo_->binaryVar(binvar);
      //constraintInfo_.push_back(*cInfo_);
      //vector of perspective constraints
      cList_.push_back(cons);

      //vector of binary variables of perspective constraint
      binVar_.push_back(binvar);
      lbc_.push_back(l_);
      ubc_.push_back(u_);

      //Count of perspective constraints
      //stats_->totalpersp += 1;

//#if (DEBUG_LEVEL >= 0) 
      //int y;
      //y=constraintInfo_.size()-1;
      //printConsInfo(&constraintInfo_[y]);
//#endif
    }
    l_.clear();
    u_.clear();
    //stats_->totalcons += 1;
    //delete cInfo_;

  }
#if SPEW 
  outfile_ = "PerspConDebug.txt";
  // This is done just to clean the output debug file.
  output_.open(outfile_.c_str());
  //generateList(p_);
  displayInfo();
  output_.close(); 
    //perspConsInfo_=&constraintInfo_;
#endif
}

bool PerspCon::evalConstraint(ConstConstraintPtr cons, VariablePtr& binvar)
{
  // Type of function considered.
  FunctionType type;
  // Function type of constraint.
  type = cons->getFunctionType();
  // Do not consider linear constraints.
  if (type == Linear) {
    return false;
  }
  // Get function of constraint.
  const FunctionPtr f = cons->getFunction(); 
  const NonlinearFunctionPtr nlf = cons->getNonlinearFunction(); 
  // add one more parameter that stores the binary variable.
  // vartypeok = true, if no. of binary variable in constraint is 
  // less than or equal to 1
  bool vartypeok = checkVarTypes(f, binvar);
  if (vartypeok ==  false) {
    return false;
  }
  // Check if the constraint is separable.
  bool issep = false;
  if (binvar == NULL) {
    issep = true;
  } else {
    //issep = true, if constraint function is separable in
    // cont. and bin variables
    issep = separable(cons, binvar);
  }
  if (issep == false) {
    return false;
  }
  //Check if any of the continuous variables of the constraint has lower bound
  // greater than zero. In this case PR is not applicable 
  //bool lbp=false;
  //lbp=checkVarsLBounds(f);
  //if (lbp==true) {
    //return false;
  //} 
  // Check which cont. variables of the function are bounded by binary.
  bool boundsok = false;
  if (binvar == NULL) {
    VarSetPtr binaries = (VarSetPtr) new VarSet();
    // Select first variable of the nonlinear part of the
    // constraint for initial binary search.
    ConstVariablePtr initvar = *(nlf->varsBegin());
    //ConstVariablePtr initvar = *(f->varsBegin());
    initialBinary(initvar, binaries);

    // If there is no binary controlling the select variable
    // then stop.
    if (binaries->size() == 0) {
      return false;
    }
    for (VarSetConstIterator it= binaries->begin(); it!=binaries->end(); ++it) {
      binvar = *it;
      // Check at least one binary varible that is controlling initvar is
      // controlling rest of the variables in the nonlinear part of the
      // constriant function
      boundsok = checkNVars(nlf, binvar);
      if (boundsok == true) {
        boundsok = checkLVars(cons, binvar);
        if (boundsok == true) {
          return true;
        }
      }
      l_.clear();
      u_.clear();
    }
  } else {
    // For each variable check if it is bounded by a binary variable.
    //boundsok = checkVarsBounds(f, binvar);
    boundsok = checkNVars(nlf, binvar);
    if (boundsok == true) {
      boundsok = checkLVars(cons, binvar);
      if (boundsok == true) {
        return true;
       }
    }
  }
  // If any variable is not bounded by binary variable, 
  // constraint is not considered.
  if (boundsok ==  false) {
    return false;
  } else {
  // to be continued.  
    return true;
  }
}

bool PerspCon::separable(ConstConstraintPtr cons, ConstVariablePtr binvar)
{

  //bool isSep=false;
  //Quadratic part should not include the binary variable.
  QuadraticFunctionPtr qf = cons->getQuadraticFunction();
  if ((qf != NULL)) {
    if (qf->hasVar(binvar) == true) {
      return false;
    }
    else {
      //Quadratic function is separable if not bilinear
      for(VariablePairGroupConstIterator it = qf->begin(); it != qf->end(); 
            ++it) {
        if (it->second != 0.0) {    
          if (it->first.first->getId() != it->first.second->getId()) {
            //function is bilinear which means not separable
            return false;
          }   
        }   
      }
      return true;
    }
  }
 // Nonlinear part should not include the binary variable.
  NonlinearFunctionPtr nlf = cons->getNonlinearFunction();
  if ((nlf != NULL)) {
    if (nlf->hasVar(binvar) == true) {
      return false;
    } else {
      //isSep=separable(nlf);
      //return isSep;
      return true;
    }
  }
  return false;
}

//bool PerspCon::checkVarsBounds(const FunctionPtr f, ConstVariablePtr binvar)
//{
  //// Current variable considered.
  //ConstVariablePtr var;
  //bool varbounded;
  //// For each variable check the bounds from these constraints.
  //// Do not consider binvar in this loop, it should not be bounded.
  //for (VarSetConstIterator it=f->varsBegin(); (it!=f->varsEnd()) && (*it!=binvar); ++it) {
    //var = *it;
    //varbounded = checkVarBounds(var, binvar);
    //// If variable is not bounded, then constraint is not a perspective constraint.
    //if (varbounded == false) {
      //return false;
    //}
  //}
  //return true;
//}

bool PerspCon::checkNVars(const NonlinearFunctionPtr nlf, ConstVariablePtr binvar)
{  
  ConstVariablePtr var;
  bool varbounded;
  // Check if remaining variable of the nonlinear constraint is controlled by
  // binvar
  for (VarSetConstIterator it=nlf->varsBegin(); it!=nlf->varsEnd(); ++it ) {
    var = *it;
    varbounded = checkVarBounds(var, binvar);
    // If variable is not bounded, then constraint is not a perspective constraint.
    if (varbounded == false) {
      return false;
    }
  }
  return true;
}

bool PerspCon::checkLVars(ConstConstraintPtr cons, ConstVariablePtr binvar)
{
  ConstVariablePtr var;
  bool varbounded;
  //bool varbounded, r = false;
  //double coeffvar;
  const LinearFunctionPtr lf = cons->getLinearFunction();
  // For each variable check the bounds from these constraints.
  // Do not consider binvar in this loop, it should not be bounded.

  //UInt c =0; // For counting number of variables not controlled by binvar
  if (lf) {
    for (VariableGroupConstIterator it=lf->termsBegin();it!=lf->termsEnd();
         ++it) {
      var = it->first;
      if (it->first == binvar) {
        continue;
      }
      // For counting number of variables not controlled by binvar
      //c =0;
      // Check if a variable of linear function is controlled by binvar
      varbounded = checkVarBounds(var, binvar);
      // If variable is not bounded, then constraint is not a perspective constraint.
      if (varbounded == false && cons->getUb()<=0) {
        //c = 1;
        return true;
        //if (c > 1) {
          //return false;
        //} else {
          //coeffvar = lf->getWeight(var);
          //if (cons->getUb() == 0 and coeffvar < 0){
            //r = true;
          //}
        //}
      }
      if (varbounded == false && cons->getUb() > 0) {
        //c = 1;
        return false;
        //if (c > 1) {
          //return false;
        //} else {
          //coeffvar = lf->getWeight(var);
          //if (cons->getUb() == 0 and coeffvar < 0){
            //r = true;
          //}
        //}
      }
      //else {
        //return false;
      //}
    }

    if ( cons->getUb() >= 0){
      return true;
    } else {
      return false;
    }
  } else {
    return true;
  }
}

//bool PerspCon::checkVarsLBounds(const FunctionPtr f)
//{
  //// Current variable considered.
  //ConstVariablePtr var;
  //// For each variable check the bounds from these constraints.
  //// Do not consider binvar in this loop, it should not be bounded.
  //double lb;
  //for (VarSetConstIterator it=f->varsBegin(); it!=f->varsEnd(); ++it) {
    //var = *it;
    //lb=0.0;
    //lb=var->getLb();
    //if (lb > 0) {
      //return true;
    //}
  //}
  //return false;
//}

bool PerspCon::checkVarBounds(ConstVariablePtr var, ConstVariablePtr binvar)
{
  // Shows if variable is upper bounded.
  bool vub = false;
  // Shows if variable is lower bounded.
  bool vlb = false;

  // If any bound is assigned from variable bounds, 
  // corresponding bound constraint will be uninitialized as below.
  // Lb bounding constraint.
  ConstConstraintPtr lbcons = (ConstConstraintPtr) new Constraint();
  // Ub bounding constraint.
  ConstConstraintPtr ubcons = (ConstConstraintPtr) new Constraint();
  ConstConstraintPtr c;
  FunctionType type;
  double coeffvar;
  double coeffbin;
  UInt numvars;
  // Iterate through each constraint in which variable var is appearing
  for (ConstrSet::iterator it= var->consBegin(); it!= var->consEnd(); ++it) {
    c = *it;
    // Function of constraint.
    const FunctionPtr f = c->getFunction();
    // Type of constraint.
    type = c->getFunctionType();
    // Consider only linear constraints.
    if (type != Linear) {
      continue;
    } 
    // Number of variables in the constraint should be two
    // and one of them is current variable and the other one is binvar.
    numvars = f->getNumVars();
    if (numvars != 2) {
      continue;
    }
    
    // Get linear function.
    const LinearFunctionPtr lf = c->getLinearFunction();

    // Coefficient of variable.
    coeffvar = lf->getWeight(var);
    // Coefficient of binary variable.
    coeffbin = lf->getWeight(binvar);
    //If this binvar or var is not in the constraint then move to next constraint
    if (coeffbin==0 || coeffvar==0) {
      continue;
    }

    // Bounds of constraint.
    //lb = cons->getLb();
    //ub = cons->getUb();
    //// Check upper and lower bounds.
    //if (coeffvar > 0) { 
      //ub=ub/coeffvar;
      //lb=lb/coeffvar;
      ////coeffbin=coeffbin/coeffvar;
    //} else {
      //dtmp=ub;
      ////MS: minus sign is removed
      //ub=lb/coeffvar;
      //lb=dtmp/coeffvar;
      ////coeffbin=coeffbin/coeffvar;
    //}
    //lb=std::max(lb, var->getLb());
    //ub=std::min(ub, var->getUb());

    //if (lbbounded==false) {
      //if (lb==0) {
        //lbcons=cons;
        //lbbounded=true;
      //}
    //}

    //if (ubbounded==false) {
      //if (ub==0) {
       //ubcons=cons;
        //ubbounded=true;
      //}
    //}

    //if (lbbounded==true && ubbounded==true) {
      //break;
    //}
   if (vlb == false) {
     //if (coeffvar < 0 && coeffbin >0 && c->getUb() == 0) {
       //vlb = true;
       //lbcons = c;
     //} 
     if (coeffvar < 0 && c->getUb() == 0) {
       vlb = true;
       lbcons = c;
     }         
     //if (coeffvar > 0 && coeffbin < 0 && c->getLb() == 0) {
       //vlb = true;
       //lbcons = c;
     //}    
     if (coeffvar > 0 && c->getLb() == 0) {
       vlb = true;
       lbcons = c;
     } 
   }

   if (vub == false) {
     if (coeffvar < 0 && c->getLb() == 0) {
       vub = true;
       ubcons = c;
     } 
     if (coeffvar > 0 && c->getUb() == 0) {
       vub = true;
       ubcons = c;
     }
   }
   
  
   if (vlb ==true && vub == true) {
     break;
   }  
  } // end of for loop to consider all constraints that has the variable.

  if (vlb == 0 && var->getLb() == 0){
     vlb = true;
   }
 
  // If variable is both bounded from up and down from binary variable, then
  // it is bounded by the binary variable.
  if ( (vub == true) && (vlb == true) ) {
    l_.push_back(lbcons->getName());
    u_.push_back(ubcons->getName());
    //cInfo_->ubCons(ubcons->getName());
    //cInfo_->contiVar(var);
    return true;
  }

  // If it comes here, variable is not bounded by binary variable.
  return false;
}


bool PerspCon::checkVarTypes(const FunctionPtr f, ConstVariablePtr& binvar)
{
  // Current variable considered.
  ConstVariablePtr var;
  // Type of variable considered.
  VariableType type;
  // Number of binary variables.
  UInt numbins = 0;
  // Number of integer binary variables.
  UInt numintbins = 0;
  // Iterate through all variables.
  for (VarSetConstIterator it=f->varsBegin(); it!=f->varsEnd(); ++it) {
    var = *it;
    type = var->getType();
    // Check the type of variable.
    switch (type) {
    case Binary:
      binvar = var;
      numbins += 1; 
      // If number of binary variables is more than one
      // Do not consider constraint for perspective cuts generation.
      if (numbins + numintbins >= 2) {
        return false;
      }
      break;
    case ImplBin:
      binvar = var;
      numbins += 1; 
      // If number of binary variables is more than one
      // Do not consider constraint for perspective cuts generation.
      if (numbins + numintbins >= 2) {
        return false;
      }
      break;
    case Integer:
      if ( (var->getLb() == 0) && (var->getUb() == 1) ) {
        binvar = var;
        numintbins += 1;
        if (numbins + numintbins >= 2) {
          return false;
        } 
      } else {
        // It is a general variable
        // Do not consider constraint.
        return false;
      }
      break;
    case Continuous: /* Do nothing.*/ 
      break;
    default:
      // If it comes to here, we have a variable which was not expected 
      // when algorithm was designed.
      return false;
    }
  }

  // If it comes here, it means all variables are continuous 
  // or there exists only one binary variable. 
  return true;
}

void PerspCon::initialBinary(ConstVariablePtr var, VarSetPtr binaries)
{
  // Current constraint considered.
  ConstConstraintPtr cons;
  FunctionType type; 
  UInt numvars;
  // Iterate through each constraint.
  for (ConstrSet::const_iterator it= var->consBegin(); it!=var->consEnd(); ++it) {
    cons = *it;
    // Function of constraint.
    const FunctionPtr f = cons->getFunction();
    // Type of function.
    type = cons->getFunctionType();
    // Only consider linear constraints.
    if (type != Linear) {
      continue;
    }
    // Number of variables should be two.
    numvars = f->getNumVars();
    if (numvars != 2) {
      continue;
    }
    // Get linear function.
    const LinearFunctionPtr lf = cons->getLinearFunction();

    // Check if the other variable is binary.
    // Iterators for variables in constraint.
    ConstVariablePtr curvar;
    VariableType vartype;
    double varlb;
    double varub;
    for (VariableGroupConstIterator itvar=lf->termsBegin(); itvar!=lf->termsEnd(); ++itvar) {
      curvar = itvar->first;
      vartype = curvar->getType();
      if (vartype == Continuous) {
        continue;
      }
      if (vartype == Binary) {
        binaries->insert(curvar);
      }
      if (vartype == ImplBin) {
        binaries->insert(curvar);
      }
      if (vartype == Integer) {
        varlb = curvar->getLb();
        varub = curvar->getUb();
        if ( (varlb == 0) && (varub==1) ) {
          binaries->insert(curvar);
        }
      }
    } // end of for for variables.

  } // end of for loop.

  //if (binaries->size() >= 1) {
    //return true;
  //} else {
    //return false;
  //}
}

bool PerspCon::getStatus()
{
  if (cList_.size() > 0){
    return true; 
  } else {
    return false;
  }
}

PerspCon::~PerspCon()
{
  // Deallocate memory.
  // If statistics is created, deallocated the memory.
  //if (stats_) {
    //delete stats_;
  //}
  env_.reset();
  p_.reset();
}

void PerspCon::displayInfo()
{
  for (UInt i= 0; i < cList_.size() ; ++i) {
    output_ << "Perspective constraint is: " << endl;
    cList_[i]->write(output_);
    output_ << "Binary variable is: " << binVar_[i]->getName() << endl;
    output_ << "Name of Lower bounding constraint: " ;
    for (UInt j=0; j < lbc_[i].size(); ++j) {
      output_ << lbc_[i][j] << ", ";
    }

    output_ << "\nName of Upper bounding constraint: ";
    for (UInt j=0; j < ubc_[i].size(); ++j) {
      output_ << ubc_[i][j] << ", ";
    }
    output_ << "\n------------------------------" << endl;
  }
  output_ << "Total no. of constraints amenable to PR " << cList_.size()
    <<  endl;

}

// Local Variables: 
// mode: c++ 
// eval: (c-set-style "k&r") 
// eval: (c-set-offset 'innamespace 0) 
// eval: (setq c-basic-offset 2) 
// eval: (setq fill-column 78) 
// eval: (auto-fill-mode 1) 
// eval: (setq column-number-mode 1) 
// eval: (setq indent-tabs-mode nil) 
// End:
