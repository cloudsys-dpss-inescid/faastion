import results


BINARY_SIZE_METRIC = "binary-size"


tika_build = results.read_build_output('../results/tika-build-image.log')
petclinic_build = results.read_build_output('../results/petclinic-build-image.log')
shopcart_build = results.read_build_output('../results/shopcart-build-image.log')

bench_results_tika_ni = results.read_benchmark_results("../results/bench-results-tika-ni.json")
bench_results_petclinic_ni = results.read_benchmark_results("../results/bench-results-petclinic-ni.json")
bench_results_shopcart_ni = results.read_benchmark_results("../results/bench-results-shopcart-ni.json")


tika_build["binary_size"] = results.get_benchmark_metric(BINARY_SIZE_METRIC, bench_results_tika_ni)
petclinic_build["binary_size"] = results.get_benchmark_metric(BINARY_SIZE_METRIC, bench_results_petclinic_ni)
shopcart_build["binary_size"] = results.get_benchmark_metric(BINARY_SIZE_METRIC, bench_results_shopcart_ni)

print("Tika: ", tika_build)
print("Petclinic: ", petclinic_build)
print("Shopcart: ", shopcart_build)
