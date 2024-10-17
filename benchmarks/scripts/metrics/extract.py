import matplotlib.pyplot as plt
import statistics

def read_numbers_from_file(file_path):
    numbers = []
    try:
        with open(file_path, 'r') as file:
            data = file.read()
            numbers_str = data.split()
            for num_str in numbers_str:
                try:
                    num_float = float(num_str)
                    numbers.append(num_float)
                except ValueError:
                    print(f"Warning: Skipping non-numeric value '{num_str}' in file '{file_path}'")
    except FileNotFoundError:
        print(f"Error: File '{file_path}' not found.")
    
    return numbers

# Define file paths
faastion_file = 'logs/experiment_20240625_172742/faastion-data.log'
faastlane_file = 'logs/experiment_20240625_172742/faastlane-data.log'
process_file = 'logs/experiment_20240625_172742/process-data.log'
isolate_file = 'logs/experiment_20240625_172742/isolate-data.log'  # Assuming this should be a different file

# Read numbers from each file
faastion = read_numbers_from_file(faastion_file)
faastlane = read_numbers_from_file(faastlane_file)
process = read_numbers_from_file(process_file)
isolate = read_numbers_from_file(isolate_file)

# Calculate medians
median_faastion = statistics.median(faastion)
median_faastlane = statistics.median(faastlane)
median_process = statistics.median(process)
median_isolate = statistics.median(isolate)

# Create box plots with tick_labels parameter
plt.figure(figsize=(10, 6))

plt.boxplot([faastion, faastlane, process, isolate],
            tick_labels=['faastion', 'faastlane', 'process', 'isolate'])

plt.title('End to End Latency Across Comparision Targets')
plt.ylabel('Time (milliseconds)')
plt.grid(True)

# Save the plot as an image file
plt.savefig('boxplot_numeric_data.png')

# Display the plot
plt.show()
