import results
import numpy as np
import matplotlib.pyplot as plt
import numbers


width = 0.15
benchmark_labels = [
    "Tika",
    "Petclinic",
    "Shopcart"
]
x = np.arange(len(benchmark_labels))

# reflect-config.json
# jni-config.json
# predefined-classes-config.json
# proxy-config.json
# serialization-config.json -- remove
# resource-config.json

def get_field_count(obj):
    if isinstance(obj, str) or isinstance(obj, numbers.Number) or isinstance(obj, bool):
        return 1
    count = 0
    if isinstance(obj, list):
        for item in obj:
            count += get_field_count(item)
    else:
        for key, value in obj.items():
            count += get_field_count(value)
    return count


def get_config_complexity(configs):
    complexities = []
    for config in configs:
        complexities.append(get_field_count(config))
    return complexities


tika_configs = results.read_config_files('../results/tika-config')
petclinic_configs = results.read_config_files('../results/petclinic-config')
shopcart_configs = results.read_config_files('../results/shopcart-config')

tika_config_complexity = get_config_complexity(tika_configs)
petclinic_config_complexity = get_config_complexity(petclinic_configs)
shopcart_config_complexity = get_config_complexity(shopcart_configs)

reflect_complexity = [
    tika_config_complexity[0],
    petclinic_config_complexity[0],
    shopcart_config_complexity[0]]

jni_complexity = [
    tika_config_complexity[1],
    petclinic_config_complexity[1],
    shopcart_config_complexity[1]]

predefined_classes_complexity = [
    tika_config_complexity[2],
    petclinic_config_complexity[2],
    shopcart_config_complexity[2]]

proxy_complexity = [
    tika_config_complexity[3],
    petclinic_config_complexity[3],
    shopcart_config_complexity[3]]

# Skip serialization config since it's always 0

resource_complexity = [
    tika_config_complexity[5],
    petclinic_config_complexity[5],
    shopcart_config_complexity[5]]

plt.rcParams.update({'font.size': 24})
fig, ax = plt.subplots()
ax.bar(x + 0*width, proxy_complexity,              width=width, hatch='/', label='Proxy',              alpha=0.75)
ax.bar(x + 1*width, predefined_classes_complexity, width=width, hatch='O', label='Predefined Classes', alpha=0.75)
ax.bar(x + 2*width, jni_complexity,                width=width, hatch='.', label='JNI',                alpha=0.75)
ax.bar(x + 3*width, resource_complexity,           width=width, hatch='-', label='Resources',          alpha=0.75)
ax.bar(x + 4*width, reflect_complexity,            width=width, hatch='*', label='Reflection',         alpha=0.75)

ax.set_ylabel('Configuration fields')
ax.set_xticks([position + width*2 for position in x], benchmark_labels)
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
ax.set_ylim(ymin=0.1, ymax=500000)
# ax.set_xlim(xmin=-.25, xmax=14.7)
fig.set_figwidth(15)
fig.set_figheight(7)

ax.legend(ncol=3, loc='upper center')
plt.savefig("aot-config.pdf", bbox_inches='tight')
plt.savefig("aot-config.png", bbox_inches='tight')
plt.show()
