//
// Created by florian on 12/1/22.
//

#ifndef RECONSTRUCT_LENGTHVALUES_H
#define RECONSTRUCT_LENGTHVALUES_H


class LengthValues {
public:
  LengthValues();

  size_t getLengthOfN() const;

  size_t getLengthOfDq() const;

  size_t getLengthOfDp() const;

  size_t getLengthOfD() const;

  size_t getLengthOfQ() const;

  size_t getLengthOfP() const;

private:
  size_t lengthOfN;
  size_t lengthOfDq;
  size_t lengthOfDp;
  size_t lengthOfD;
  size_t lengthOfQ;
  size_t lengthOfP;

  void loadParameters();
};


#endif //RECONSTRUCT_LENGTHVALUES_H
