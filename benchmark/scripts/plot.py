import json
import seaborn as sns
import re
import sys
import os
import os.path
import pandas as pd
import tinydb
from matplotlib import pyplot as plt
import pprint

DEFAULT_STYLE = {
    'secondary': 'style'
}

pp = pprint.PrettyPrinter()


class ExperimentPlotter(object):
    def __init__(self, base_path, data, config):
        self._data = data
        self._config = config
        self._base_path = base_path

    def _build_query(self, qspec):
        if isinstance(qspec, list):
            # implicit and
            dummy_spec = {'sub': qspec}
            return self._build_and_query(dummy_spec)

        kind = qspec.get('kind', 'simple')
        if kind == 'and':
            return self._build_and_query(qspec)
        elif kind == 'or':
            return self._build_or_query(qspec)
        elif kind == 'set':
            return self._build_set_query(qspec)
        else:
            assert kind == 'simple'
            return self._build_simple_query(qspec)

    def _build_set_query(self, qspec):
        q_builder = tinydb.Query()
        composed_q = None

        field = qspec['field']
        for item in qspec['set']:
            print(f"Item: {item}")
            q = getattr(q_builder, field).search(item)

            if composed_q is None:
                composed_q = q
            else:
                composed_q |= q

        return composed_q

    def _build_and_query(self, qspec):
        composed_q = None

        for sub_spec in qspec['sub']:
            sub_q = self._build_query(sub_spec)
            if composed_q is None:
                composed_q = sub_q
            else:
                composed_q &= sub_q

        return composed_q

    def _build_or_query(self, qspec):
        composed_q = None

        for sub_spec in qspec['sub']:
            sub_q = self._build_query(sub_spec)
            if composed_q is None:
                composed_q = sub_q
            else:
                composed_q |= sub_q

        return composed_q

    def _build_simple_query(self, qspec):
        q_builder = tinydb.Query()

        field = qspec['field']
        if 'match' in qspec:
            q = getattr(q_builder, field).search(qspec['match'])
        elif 'min' in qspec:
            q = getattr(q_builder, field).test(
                lambda s: float(s) >= float(qspec['min']))
        elif 'max' in qspec:
            q = getattr(q_builder, field).test(
                lambda s: float(s) <= float(qspec['min']))

        if qspec.get('negate', False):
            q = ~q

        return q

    def _get_data(self):
        q = self._build_query(self._config['filters'])

        return list(self._data.search(q))

    def plot(self):
        plt.clf()

        d = self._get_data()

        df = pd.DataFrame(d)

        x_axis = self._config.get('x_axis', 'base_size')
        x_power = int(self._config.get('x_power', 6))
        x_label = self._config.get('x_label', 'Size')
        if x_power > 1:
            df[x_axis] = df[x_axis].apply(lambda x: x / (10**x_power))
            x_label += f" $(\\times 10^{{{x_power}}})$"

        y_axis = self._config.get('y_axis', 'cpu_time')
        y_power = int(self._config.get('y_power', 6))
        y_label = self._config.get('y_label', 'Time (ns)')
        if y_power > 1:
            df[y_axis] = df[y_axis].apply(lambda y: y / (10**y_power))
            y_label += f" $(\\times 10^{{{y_power}}})$"

        plot = sns.lineplot(x=x_axis,
                            y=y_axis,
                            hue=self._config.get('hue', 'full_algo'),
                            data=df)

        plot.set(xlabel=x_label,
                 ylabel=y_label)

        output_file = os.path.join(self._base_path, self._config['filename'])
        plot.get_figure().savefig(output_file)


class DataReader(object):
    name_re = re.compile(
        r'(?P<group>[^\s]+) :: (?P<experiment>[^\s]+) :: (?P<algorithm>[^[/]+)(\[(?P<algopts>[^\]]*)\])?/(?P<base_size>\d+)/(?P<experiment_size>\d+)/\d+/manual_time')

    def __init__(self, json_data):
        self._data = []
        self._json_data = json_data

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
                'algorithm': m.group('algorithm'),
                'algopts': m.group('algopts'),
                'full_algo': "{} [{}]".format(m.group('algorithm'),
                                              m.groupdict().get('algopts', 'foo'))
            }

            self._data.append(d)

    def get(self):
        db = tinydb.TinyDB(storage=tinydb.storages.MemoryStorage)
        for entry in self._data:
            db.insert(entry)
        return db


if __name__ == '__main__':
    data_filename = sys.argv[1]
    cfg_filename = sys.argv[2]
    output_dir = sys.argv[3]

    with open(data_filename, 'r') as json_file:
        json_data = json.load(json_file)

    with open(cfg_filename, 'r') as cfg_file:
        cfg = json.load(cfg_file)

    reader = DataReader(json_data)
    data = reader.get()
    for plot_cfg in cfg:
        p = ExperimentPlotter(output_dir, data, plot_cfg)
        p.plot()
