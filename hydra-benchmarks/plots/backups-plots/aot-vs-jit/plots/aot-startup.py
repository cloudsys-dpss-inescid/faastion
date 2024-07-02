import results
import numpy as np
import matplotlib.pyplot as plt


THROUGHPUT_METRIC = "app-startup"
width = 0.3
benchmark_labels = [
    "Tika",
    "Petclinic",
    "Shopcart"
]
x = np.arange(len(benchmark_labels))


bench_results_tika_ni = results.read_benchmark_results("../results/bench-results-tika-ni.json")
bench_results_tika_jvm = results.read_benchmark_results("../results/bench-results-tika-jvm.json")
bench_results_petclinic_ni = results.read_benchmark_results("../results/bench-results-petclinic-ni.json")
bench_results_petclinic_jvm = results.read_benchmark_results("../results/bench-results-petclinic-jvm.json")
bench_results_shopcart_ni = results.read_benchmark_results("../results/bench-results-shopcart-ni.json")
bench_results_shopcart_jvm = results.read_benchmark_results("../results/bench-results-shopcart-jvm.json")

jvm_startup = [
    results.get_startup_metric(bench_results_tika_jvm),
    results.get_startup_metric(bench_results_petclinic_jvm),
    results.get_startup_metric(bench_results_shopcart_jvm)]

ni_startup = [
    results.get_startup_metric(bench_results_tika_ni),
    results.get_startup_metric(bench_results_petclinic_ni),
    results.get_startup_metric(bench_results_shopcart_ni)]

plt.rcParams.update({'font.size': 13})
fig, ax = plt.subplots()
ax.bar(x + 0*width, jvm_startup, width=width, hatch='*', label='JVM',           alpha=0.75)
ax.bar(x + 1*width, ni_startup,  width=width, hatch='O', label='Native Image',  alpha=0.75)

ax.set_ylabel('Startup latency (s)')
ax.set_xticks([position + width/2 for position in x], benchmark_labels)
ax.set_axisbelow(True)
plt.grid(axis = 'y', linestyle = '--', linewidth = 0.25)
plt.xticks(rotation = 35)
# plt.tick_params(
#     axis='x',          # changes apply to the x-axis
#     which='both',      # both major and minor ticks are affected
#     bottom=False,      # ticks along the bottom edge are off
#     top=False,         # ticks along the top edge are off
#     labelbottom=False) # labels along the bottom edge are off

ax.set_yscale('log')
ax.set_ylim(ymin=0.01, ymax=10000)
# ax.set_xlim(xmin=-.25, xmax=14.7)
fig.set_figwidth(4)
fig.set_figheight(3)

ax.legend(ncol=5, loc='upper center')
plt.savefig("aot-startup.pdf", bbox_inches='tight')
plt.savefig("aot-startup.png", bbox_inches='tight')
plt.show()
