import json
import seaborn as sns
import re
import sys
import os
import pandas as pd
from matplotlib import pyplot as plt

DEFAULT_STYLE = {
    'secondary': 'style'
}


class ExperimentPlotter(object):
    def __init__(self, data, x='base', style=DEFAULT_STYLE):
        self._raw_d = data
        self._x_dimension = x
        self._secondary_key = 'experiment_size' if self._x_dimension == 'base' else 'base_size'
        self._style = style

        self._prepare()

    def _prepare(self):
        self._d = list(self._raw_d)

        # Figure out whether the size that's not on the x axis has more than one value
        self._secondary_x = False
        value = self._raw_d[0][self._secondary_key]
        for entry in self._raw_d:
            if entry[self._secondary_key] != value:
                self._secondary_x = True
                break

        if self._secondary_x and self._style['secondary'] != 'style':
            # We need to incorporate the secondary size into the value used for hue
            for entry in self._d:
                self._d['algorithm'] += "[" + \
                    str(entry[self._secondary_x]) + "]"

    def plot(self, filename):
        plt.clf()
        if self._secondary_x and self._style['secondary'] == 'style':
            style = self._secondary_key
        else:
            style = None

        df = pd.DataFrame(self._d)

        plot = sns.lineplot(x='base_size',
                            y='cpu_time',  # TODO make configurable
                            hue='algorithm',
                            style=style,
                            data=df)

        plot.get_figure().savefig(filename)


class DataReader(object):
    name_re = re.compile(
        r'(?P<group>[^\s]+) :: (?P<experiment>[^\s]+) :: (?P<algorithm>[^/]+)/(?P<base_size>\d+)/(?P<experiment_size>\d+)')

    def __init__(self, json_data):
        self._data = []
        self._json_data = json_data

        self._experiments = set()

        self._process()

    def _process(self):
        for entry in self._json_data['benchmarks']:
            if entry['run_type'] == 'aggregate':
                continue

            name = entry['name']
            m = DataReader.name_re.match(name)
            if not m:
                print("Could not parse experiment name:")
                print(name)
                exit(-1)

            d = {
                'cpu_time': float(entry['cpu_time']),
                'real_time': float(entry['real_time']),
                'iterations': float(entry['iterations']),
                'time_unit': entry['time_unit'],  # TODO convert between them!
                'base_size': float(m.group('base_size')),
                'experiment_size': float(m.group('experiment_size')),
                'group': m.group('group'),
                'experiment': m.group('experiment'),
                'algorithm': m.group('algorithm')
            }

            self._experiments.add((m.group('group'), m.group('experiment')))

            self._data.append(d)

    def get_experiments(self):
        return self._experiments

    def get(self, experiment=None):
        if not experiment:
            return self._data
        else:
            return list(filter(lambda d: d['group'] == experiment[0] and d['experiment'] == experiment[1],
                               self._data))


if __name__ == '__main__':
    json_filename = sys.argv[1]
    output_dir = sys.argv[2]

    with open(json_filename, 'r') as json_file:
        json_data = json.load(json_file)

    reader = DataReader(json_data)
    experiments = reader.get_experiments()
    for experiment in experiments:
        group_name, experiment_name = experiment
        experiment_data = reader.get(experiment)
        plotter = ExperimentPlotter(experiment_data)

        output_filename = "{}_{}.pdf".format(group_name, experiment_name)
        output_path = os.path.join(output_dir, output_filename)
        plotter.plot(output_path)
