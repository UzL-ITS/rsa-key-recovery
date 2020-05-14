//
// Created by florian on 16.09.20.
//

#include "ValidationValuesFactory.h"

std::shared_ptr<ValidationValues> ValidationValuesFactory::validationValues;

std::shared_ptr<ValidationValues> ValidationValuesFactory::getValidationValues() {
  if (validationValues == nullptr) {
    validationValues = std::make_shared<ValidationValues>();
  }

  return validationValues;
}
