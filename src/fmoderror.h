#ifndef FMODERROR_H
#define FMODERROR_H

#define FMODWarning(_result) FMODWarning_fn(_result, __FILE__, __LINE__)

void FMODError(int result);
bool FMODWarning_fn(int result, const char *file, int line);

#endif // FMODERROR_H
