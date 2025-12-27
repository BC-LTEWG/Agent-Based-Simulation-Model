#include <memory>
#include <vector>

typedef std::vector<double> Row;
typedef std::vector<std::unique_ptr<Row>> Matrix;

void transpose(Matrix *);
void reduced_row_echelon(Matrix *);
