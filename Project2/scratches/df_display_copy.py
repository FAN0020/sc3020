# %%
# import libraries
import os
import pandas as pd
import dotenv
import psycopg2
import pygraphviz as pgv


# define functions

# function to run sql to retrieve QEP of query.
def retrieve_qep(sql):
    # adds explain line to retrieve query details including QEP
    sql_exp = "explain (analyze, costs, verbose, buffers, format json) " + sql

    # execute query
    qep = executeQuery(sql_exp)

    # edit result to contain dictionary of query plan.
    while (type(qep) != dict):
        qep = qep[0]

    return qep


# function to run sql query and rerieve results (dataframe)
def retrieve_result(sql, columns):
    # execute query
    result = executeQuery(sql)
    df = pd.DataFrame(result, columns=columns)
    return df


dotenv.load_dotenv()


# a function to connect to DB and execute query, returning results of query
def executeQuery(sql):
    # connect to database.
    # edit values according to your own database.
    conn = psycopg2.connect(database=os.getenv("DATABASE"),
                            host="localhost",
                            user="postgres",
                            password=os.getenv("PASSWORD"),
                            port="5432")

    cursor = conn.cursor()

    # run query and retrieve results
    cursor.execute(sql)
    result = cursor.fetchall()

    conn.commit()
    conn.close()

    # return results
    return result


# break down current node and get node details.
def breakdown(cur_node):
    results = {"node": {}, "vertices": set(), "edges": set(), "sql": {}}

    # get details of current node.
    node_details = getNodeDetails(cur_node)
    results["node"][node_details["string"]] = node_details
    results["vertices"].add(node_details["string"])
    results['sql'][node_details["string"]] = getSQL(node_details)

    # break down child node
    if "Plans" in cur_node:
        child_siblings = []
        for child_node in cur_node["Plans"]:
            child_node_details = getNodeDetails(child_node)
            results["edges"].add((node_details["string"], child_node_details["string"]))
            child_results = breakdown(child_node)
            update_results(results, child_results, node_details, child_node_details, child_siblings)

    return results

def update_results(results, child_results, node_details, child_node_details, child_siblings):
    results["node"].update(child_results["node"])
    results["vertices"].update(child_results["vertices"])
    results["edges"].update(child_results["edges"])
    results["sql"].update(child_results["sql"])
    results['sql'][node_details["string"]] = update_sql(results['sql'][node_details["string"]],
                                                          results['sql'][child_node_details["string"]])
    results["node"][child_node_details["string"]]['siblings'] = child_siblings
    child_siblings.append(child_node_details["string"])

# def get_node_details(node):
#     details = {"Node Type": node["Node Type"]}
#     details["Cond"] = get_key_values(node, "cond")
#     details["Filter"] = get_key_values(node, "Filter")
#     details["Group Key"] = get_key_values(node, "Group Key")
#     details["Sort Key"] = get_key_values(node, "Sort Key")
#     details["Method"] = get_key_values(node, "method")
#     details["Relation Name"] = get_key_values(node, "Relation Name")
#     details["Alias"] = get_key_values(node, "Alias")
#     details["Output"] = get_key_values(node, "Output")
#
#     detail_string = f"{details['Node Type']}\n" \
#                     f"Cond: {', '.join(details['Cond'])}\n" \
#                     f"Filter: {', '.join(details['Filter'])}\n" \
#                     f"Group Key: {', '.join(details['Group Key'])}\n" \
#                     f"Sort Key: {', '.join(details['Sort Key'])}\n" \
#                     f"Method: {', '.join(details['Method'])}\n" \
#                     f"Relation Name: {', '.join(details['Relation Name'])}\n" \
#                     f"Alias: {', '.join(details['Alias'])}\n" \
#                     f"Output: {', '.join(details['Output'])}"
#
#     details["string"] = detail_string
#     return details

def getNodeDetails(node):
    details = {}

    details["Node Type"] = node["Node Type"]
    detailString = str(details["Node Type"])

    values = get_key_values(node, "cond")
    if len(values) > 0:
        details["Cond"] = values
        detailString += "\nby " + ", ".join(values)

    if "Filter" in node:
        details["Filter"] = node["Filter"]
        detailString += "\nFilter: " + details["Filter"]

    if "Group Key" in node:
        details["Group Key"] = node["Group Key"]
        detailString += "\nGroup by " + ", ".join(details["Group Key"])

    if "Sort Key" in node:
        details["Sort Key"] = node["Sort Key"]
        detailString += "\nSort by " + ", ".join(details["Sort Key"])

    values = get_key_values(node, "method")
    if len(values) > 0:
        details["Method"] = values
        detailString += "\nUsing " + ", ".join(values)

    if "Relation Name" in node:
        details["Relation Name"] = node["Relation Name"]
        detailString += "\nRelation: " + ", ".join(details["Relation Name"])

    if "Alias" in node:
        details["Alias"] = node["Alias"]
        detailString += "\nAlias: " + ", ".join(details["Alias"])

    if "Output" in node:
        details["Output"] = node["Output"]
        detailString += "\nOutput: " + ", ".join(details["Output"])

    details["string"] = detailString
    return details

def getSQL(nodeDetails):
    sql = {
        'select': to_list(nodeDetails.get('Output')),
        'relation': to_list(nodeDetails.get('Alias', nodeDetails.get('Relation Name'))),
        'where': to_list(nodeDetails.get('Cond')) + to_list(nodeDetails.get('Filter')),
        'order by': to_list(nodeDetails.get('Sort Key')),
        'group by': to_list(nodeDetails.get('Group Key')),
        'method': to_list(nodeDetails.get('Method'))
    }
    sql['where'] = list(set(sql['where']))
    return sql

def concatRelationAlias(relation, alias):
    if relation == alias:
        return relation
    else:
        return relation + " " + alias


def getIntermediateResults(results):
    for node in results['node']:
        result = get_query_results(results['sql'][node], results, node)
        if result is not None:
            accessResult = result[1]
            result  = result[0]
        for col in result.columns:
            if 'ctid' in col:
                result['block'] = result[col].apply(lambda x: int((x.replace("(", "").replace(")", "")).split(",")[0]))
                result['tuple'] = result[col].apply(lambda x: int((x.replace("(", "").replace(")", "")).split(",")[1]))
                results['sql'][node]['resultBlockAccess'] = result.groupby('block')['tuple'].apply(list)
                break

        results['sql'][node]["result"] = result

        print(type(accessResult))
        if type(accessResult) != type(None):
            for col in result.columns:
                if 'ctid' in col:
                    accessResult['block'] = accessResult[col].apply(
                        lambda x: int((x.replace("(", "").replace(")", "")).split(",")[0]))
                    accessResult['tuple'] = accessResult[col].apply(
                        lambda x: int((x.replace("(", "").replace(")", "")).split(",")[1]))
                    results['sql'][node]["accessResultBlockAccess"] = accessResult.groupby('block')['tuple'].apply(list)
                    break
        elif 'Relation Name' in results["node"][node]:
            results['sql'][node]["RelationBlocks"] = executeQuery(
                "SELECT relpages FROM pg_class WHERE relname = '" + results["node"][node]["Relation Name"] + "';")[0][0]

    return results


# a function get details of a node.
def get_key_values(node_dict, search_key):
    result = []
    values = [value for key, value in node_dict.items() if search_key in key.lower()]
    for val in values:
        if isinstance(val, list):
            result.extend(val)
        else:
            result.append(val)
    return result

def concatenate_relation(a, b):
    return a if a is b else f"{a} {b}"

# def get_sql(node_details):
#     sql = {
#         'select': to_list(node_details.get('Output')),
#         'relation': [concatenate_relation(a, b) for a, b in zip(to_list(node_details.get('Relation Name')),
#                                                                to_list(node_details.get('Alias')))],
#         'where': to_list(node_details.get('Cond')) + to_list(node_details.get('Filter')),
#         'order by': to_list(node_details.get('Sort Key')),
#         'group by': to_list(node_details.get('Group Key'))
#     }
#     sql['where'] = list(set(sql['where']))
#     return sql

def update_sql(node_sql, child_node_sql):
    for value in child_node_sql:
        if value not in {'result', 'select'}:
            node_sql[value].extend(child_node_sql[value])
            node_sql[value] = list(set(node_sql[value]))
    return node_sql

def get_query_results(sql_details, results, node_name):
    try:
        if 'Relation Name' in results['node'][node_name]:
            if type(results['node'][node_name]['Alias']) is str:
                sql_details['select'].insert(0, f'{results["node"][node_name]["Alias"]}.ctid')
        sql_query = create_query(sql_details)
        result = retrieve_result(sql_query, sql_details['select'])
    except Exception as e:
        print(f"Error executing query: {e}")
        result = None

    # Access sibling information only if 'siblings' key is present
    if 'siblings' in results['node'][node_name]:
        for sibling_node in reversed(results['node'][node_name]['siblings']):
            is_updated = False
            relations = results["sql"][sibling_node]["relation"]
            for relation in relations:
                if relation in sql_details['relation']:
                    continue
                for filter_val in sql_details['where']:
                    if relation in filter_val:
                        sql_details['relation'].extend(results["sql"][sibling_node]["relation"])
                        sql_details['relation'] = list(set(sql_details['relation']))
                        sql_details['where'].extend(results["sql"][sibling_node]["where"])
                        sql_details['where'] = list(set(sql_details['where']))
                        is_updated = True
                        break
                if is_updated:
                    break

        sql_query = create_query(sql_details)
        result = retrieve_result(sql_query, sql_details['select'])

    return result

def create_query(sql_details):
    return " ".join([
        f"select {','.join(sql_details['select'])}",
        f"from {','.join(sql_details['relation'])}" if sql_details['relation'] else "",
        f"where {' AND '.join(sql_details['where'])}" if sql_details['where'] else "",
        f"group by {','.join(sql_details['group by'])}" if sql_details['group by'] else "",
        f"order by {','.join(sql_details['order by'])}" if sql_details['order by'] else ""
    ]).strip().replace("PARTIAL", "")

# Helper function to ensure a value is a list
def to_list(value):
    return [value] if isinstance(value, str) else value if isinstance(value, list) else []


def plotQEP(results):
    G = pgv.AGraph(directed=True)
    G.add_nodes_from(results['vertices'])
    G.add_edges_from(results['edges'])

    G.write('test.dot')

    # create a png file
    G.layout(prog='dot')  # use dot
    G.draw('file.png')


# %%
sql = '''
select
  supp_nation,
  cust_nation,
  l_year,
  sum(volume) as revenue
from
  (
    select
      n1.n_name as supp_nation,
      n2.n_name as cust_nation,
      DATE_PART('YEAR',l_shipdate) as l_year,
      l_extendedprice * (1 - l_discount) as volume
    from
      supplier,
      lineitem,
      orders,
      customer,
      nation n1,
      nation n2
    where
      s_suppkey = l_suppkey
      and o_orderkey = l_orderkey
      and c_custkey = o_custkey
      and s_nationkey = n1.n_nationkey
      and c_nationkey = n2.n_nationkey
      and (
        (n1.n_name = 'FRANCE' and n2.n_name = 'GERMANY')
        or (n1.n_name = 'GERMANY' and n2.n_name = 'FRANCE')
      )
      and l_shipdate between '1995-01-01' and '1996-12-31'
      and o_totalprice > 100
      and c_acctbal > 10
  ) as shipping
group by
  supp_nation,
  cust_nation,
  l_year
order by
  supp_nation,
  cust_nation,
  l_year;
'''


# %%
def dora_(sql):
    qep = retrieve_qep(sql)
    results = breakdown(qep["Plan"])
    results['final'] = retrieve_result(sql, qep["Plan"]["Output"])
    results = getIntermediateResults(results)

    def get_results(node):
        step = []
        result = []
        for _ in node['sql'].keys():
            temp_res = []
            step.append(_)
            temp_res.append(node['sql'][_]['result'])
            temp_res.append(node['sql'][_].get('accessResult', ""))
            temp_res.append(node['sql'][_].get('resultBlockAccess', ""))
            temp_res.append(node['sql'][_].get('accessResultBlockAccess', ""))
            result.append(temp_res)

        return {k.split("\n")[0] + f" {_ + 1}": v for _, (k, v) in enumerate(zip(step, result))}

    return get_results(results)


res_1 = dora_(sql)

"""SEPARATOR HERE"""
from PyQt5 import QtCore
import pandas as pd


class DataFrameModel(QtCore.QAbstractTableModel):
    DtypeRole = QtCore.Qt.UserRole + 1000
    ValueRole = QtCore.Qt.UserRole + 1001

    def __init__(self, df=pd.DataFrame(), parent=None):
        super(DataFrameModel, self).__init__(parent)
        self._dataframe = df

    def setDataFrame(self, dataframe):
        self.beginResetModel()
        self._dataframe = dataframe.copy()
        self.endResetModel()

    def dataFrame(self):
        return self._dataframe

    dataFrame = QtCore.pyqtProperty(pd.DataFrame, fget=dataFrame, fset=setDataFrame)

    @QtCore.pyqtSlot(int, QtCore.Qt.Orientation, result=str)
    def headerData(self, section: int, orientation: QtCore.Qt.Orientation, role: int = QtCore.Qt.DisplayRole):
        if role == QtCore.Qt.DisplayRole:
            if orientation == QtCore.Qt.Horizontal:
                return self._dataframe.columns[section]
            else:
                return str(self._dataframe.index[section])
        return QtCore.QVariant()

    def rowCount(self, parent=QtCore.QModelIndex()):
        if parent.isValid():
            return 0
        return len(self._dataframe.index)

    def columnCount(self, parent=QtCore.QModelIndex()):
        if parent.isValid():
            return 0
        return self._dataframe.columns.size

    def data(self, index, role=QtCore.Qt.DisplayRole):
        if not index.isValid() or not (0 <= index.row() < self.rowCount() \
                                       and 0 <= index.column() < self.columnCount()):
            return QtCore.QVariant()

        row = index.row()
        col = index.column()
        val = self._dataframe.iloc[row, col]

        if role == QtCore.Qt.DisplayRole:
            return str(val)
        elif role == DataFrameModel.ValueRole:
            return val
        if role == DataFrameModel.DtypeRole:
            dt = self._dataframe.dtypes[col]
            return dt
        return QtCore.QVariant()

    def roleNames(self):
        roles = {
            QtCore.Qt.DisplayRole: b'display',
            DataFrameModel.DtypeRole: b'dtype',
            DataFrameModel.ValueRole: b'value'
        }
        return roles


from PyQt5.QtWidgets import (
    QApplication, QTableView, QTabWidget
)
import sys


class DataFrameTab(QTabWidget):
    def __init__(self, data_frame_getters, names):
        super().__init__()

        for get_df, name in zip(data_frame_getters, names):
            df = get_df()
            view = QTableView()
            model = DataFrameModel(df)
            view.setModel(model)
            view.setStyleSheet("""
                QTableView {
                    selection-background-color: #E0E0E0;
                    font: 11pt "Calibri";
                }
                QHeaderView::section {
                    background-color: #F0F0F0;
                    padding: 4px;
                    border: 1px solid #D0D0D0;
                    font-size: 10pt;
                }
            """)
            self.addTab(view, name)

import re

if __name__ == "__main__":
    app = QApplication(sys.argv)

    # Sample data frames and names
    data_frame_getters = [lambda df=item[0]: df for item in res_1.values()]
    names = [re.compile(r"\d+").sub("", _) for _ in res_1.keys()]

    data_frame_tab = DataFrameTab(data_frame_getters, names)
    data_frame_tab.resize(800, 600)
    data_frame_tab.show()

    sys.exit(app.exec_())
