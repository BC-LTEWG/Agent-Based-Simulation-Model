#include <memory>
#include <vector>

typedef std::vector<double> Row;
typedef std::vector<std::unique_ptr<Row>> Matrix;

void transpose(Matrix *);
void reduced_row_echelon_form(Matrix *, size_t, size_t);
void augment(Matrix * matrix);
std::unique_ptr<Matrix> inverse(Matrix *);
void round_to_zero(Matrix *, size_t, size_t);
