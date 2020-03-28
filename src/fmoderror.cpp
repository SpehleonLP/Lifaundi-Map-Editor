#include "fmoderror.h"
#include <iostream>
#include <stdexcept>

#ifndef _WIN32
#include <fmod/fmod_errors.h>
#else
#include <fmod_errors.h>
#endif

bool FMODWarning_fn(int result, const char *file, int line)
{
	if(result == FMOD_OK)
		return false;

	std::cerr << "FMOD Warning"
		<< "in" << file << ":" << line << "\n"
		<< FMOD_ErrorString((FMOD_RESULT) result);
	return true;
}

void FMODError(int result)
{
	if(result != FMOD_OK)
		throw std::runtime_error(FMOD_ErrorString((FMOD_RESULT) result));
}
