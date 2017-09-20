import gdb

class NodePrinter(object):
    def __init__(self, node):
        self.val = node
        self.depth = 0

    def to_string(self):
        return "RBTreeNode @ " + str(self.val.address)

    def children(self):
        return [
            ('foo', 'left'),
            ('foo', self.val['_rbt_left'].dereference() if self.val['_rbt_left'] else 'empty'),
            ('foo', 'right'),
            ('right', self.val['_rbt_right'].dereference() if self.val['_rbt_right'] else 'empty')
        ]

    def display_hint(self):
        return "map"

class TreePrinter(object):
    def __init__(self, val):
        self.val = val

    def to_string(self):
        return "RBTree @ " + str(self.val.address)

    def children(self):
        return [
            ("root", self.val['root'].dereference())
        ]

    def display_hint(self):
        return "string"



print(" ===== Registering Ygg Printers ===== ")

def build_pretty_printer():
    pp = gdb.printing.RegexpCollectionPrettyPrinter(
        "ygg")
    pp.add_printer('TreePrinter', '^ygg::RBTree<.*$', TreePrinter)
    pp.add_printer('NodePrinter', '^ygg::RBTreeNodeBase<.*$', NodePrinter)
    return pp

gdb.printing.register_pretty_printer(
    gdb.current_objfile(),
    build_pretty_printer())
