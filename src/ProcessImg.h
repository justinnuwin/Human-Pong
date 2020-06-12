#pragma once

#include "BackgroundSub.hpp"

class ProcessImg
{
public:
   ProcessImg();
   std::vector<uchar> Process(std::vector<uchar> left, std::vector<uchar> right);

private:
   BackgroundSub* bSub;
};
