// CG Assignment 1 Exercise 3 WS 17/18

#include <fstream>
#include <iostream>
#include <sstream>
#include <vector>

#include "MatrixInt.hpp"


/** Splits a string into substrings seperated by whitespaces.*/
void tokenize(const std::string& line, std::vector<std::string>& tokens)
{
	std::stringstream iss(line);
	std::string token;
	while (iss >> token)
		tokens.push_back(token);
}

/** Loads the values of a matrix from a file and stores them in the given matrix object.*/
bool loadMatrix(std::string const& filename, MatrixInt& matrix)
{
	unsigned int rowCount = 0;
	unsigned int columnCount = 0;

	std::ifstream inFile;
	inFile.open(filename, std::ifstream::in);

	if (!inFile.good())
		return false;

	std::vector<std::vector<std::string>> lines; // stores contents of file

												 // Read file and query number of rows and columns
	while (inFile.good()) {

		std::string line;
		getline(inFile, line);

		if (line == "")
			break;

		std::vector<std::string> tokens;
		tokenize(line, tokens);

		columnCount = static_cast<unsigned int>(tokens.size());
		rowCount++;

		lines.push_back(tokens);
	}

	// Intialization of the Matrix
	matrix = MatrixInt(rowCount, columnCount);

	// Copy values to matrix
	for (size_t currentRow = 0; currentRow<rowCount; currentRow++)
	{
		for (size_t currentColumn = 0; currentColumn<columnCount; currentColumn++)
		{
			int value = stoi(lines[currentRow][currentColumn], 0, 10);

			///////
			// TODO
			// Write the read value to the correct location of matrix
			// data storage (Row: currentRow, Column: currentColumn)

		}
	}

	return true;
}

bool writeMatrix(std::string const& filename, MatrixInt const& matrix)
{
	unsigned int rowCount = matrix.getRowCount();
	unsigned int columnCount = matrix.getColumnCount();

	std::ofstream outFile;
	outFile.open(filename, std::ofstream::out);

	if (!outFile.good()) {
		return false;
	}
	else {

		for (size_t i = 0; i < rowCount; ++i)
		{
			for (size_t j = 0; j < columnCount; ++j)
			{
				outFile << matrix.data()[i*columnCount + j] << " ";
			}
			outFile << std::endl;
		}
	}

	return true;
}

int main()
{
	MatrixInt matrix1;
	MatrixInt matrix2;

	if (loadMatrix("matrix1.txt", matrix1) &&
		loadMatrix("matrix2.txt", matrix2))
	{
		MatrixInt resultMatrix = matrix1 * matrix2;
		writeMatrix("matrix3.txt", resultMatrix);
	}
	else
	{
		std::cout << "Matrixdateien nicht gefunden" << std::endl;
	}

	return 0;
}
