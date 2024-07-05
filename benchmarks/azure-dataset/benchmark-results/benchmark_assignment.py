import csv
import argparse

def parse_args():
	parser = argparse.ArgumentParser()
	parser.add_argument(
 		"-f", 
		"--num-functions", 
		type=int, 
		dest="num_functions", 
		default=5,
		help="Number of functions"
	)
	parser.add_argument(
		"-i",
		"--input-file",
		type=str,
		dest="input_file",
		default="input.csv",
		help="Input file path",
	)
	parser.add_argument(
		"-o",
		"--output-file",
		type=str,
		dest="output_file",
		default="output.csv",
		help="Output file path",
	)

	args = parser.parse_args()
	return args.num_functions, args.input_file, args.output_file 


def read_csv(filename):
	data = []
	with open(filename, 'r', newline='') as csvfile:
		csv_reader = csv.reader(csvfile)
		for row in csv_reader:
			data.append(row)
	return data


def main():
	num_functions, input_file, output_file = parse_args()

	csv_data = read_csv(input_file)
	first_timestamp = int(csv_data[1][4])

	with open(output_file, 'w') as file:
		for value in csv_data[1:]:
			function_number = str(int(value[1], 16) % num_functions)
			timestamp = str(int(value[4]) - first_timestamp)
			file.write(function_number + "," + timestamp + "\n")


if __name__ == "__main__":
	main()
