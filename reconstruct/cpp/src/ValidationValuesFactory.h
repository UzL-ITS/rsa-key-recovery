//
// Created by florian on 16.09.20.
//

#ifndef RECONSTRUCT_VALIDATIONVALUESFACTORY_H
#define RECONSTRUCT_VALIDATIONVALUESFACTORY_H


#include <memory>
#include "ValidationValues.h"

class ValidationValuesFactory {

public:
  static std::shared_ptr<ValidationValues> getValidationValues();
private:
  static std::shared_ptr<ValidationValues> validationValues;
};


#endif //RECONSTRUCT_VALIDATIONVALUESFACTORY_H
