# %%
# import libraries
import os

import dotenv
import psycopg2
import pandas as pd
import networkx as nx
import pygraphviz as pgv


# define functions

# function to run sql to retrieve QEP of query.
def retrieveQEP(sql):
    # adds explain line to retrieve query details including QEP
    sql_exp = "explain (analyze, costs, verbose, buffers, format json) " + sql

    # execute query
    qep = executeQuery(sql_exp)

    # edit result to contain dictionary of query plan.
    while (type(qep) != dict):
        qep = qep[0]

    return qep


# function to run sql query and rerieve results (dataframe)
def retrieveResult(sql, columns):
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


def getIntermediateResults(results):
    for node in results['node']:
        result, accessResult = getQueryResults(results['sql'][node], results, node)
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

# break down current node and get node details.
def breakdown(curNode):
    results = {}
    results["node"] = {}
    results["vertices"] = set()
    results['edges'] = set()
    results["sql"] = {}

    # get details of current node.
    nodeDetails = getNodeDetails(curNode)
    # print(nodeDetails)
    results["node"][nodeDetails["string"]] = nodeDetails
    results["vertices"].add(nodeDetails["string"])
    results['sql'][nodeDetails["string"]] = getSQL(nodeDetails)

    # break down child node
    if "Plans" in curNode:
        childSiblings = []
        for node in curNode["Plans"]:
            childNodeDetails = getNodeDetails(node)
            results["edges"].add((nodeDetails["string"], childNodeDetails["string"]))
            child_results = breakdown(node)
            results["node"].update(child_results["node"])
            results["vertices"].update(child_results["vertices"])
            results["edges"].update(child_results["edges"])
            results["sql"].update(child_results["sql"])
            results['sql'][nodeDetails["string"]] = updateSQL(results['sql'][nodeDetails["string"]],
                                                              results['sql'][childNodeDetails["string"]])
            results["node"][childNodeDetails["string"]]['siblings'] = childSiblings
            childSiblings.append(childNodeDetails["string"])

    return results





# a function get details of a node.
def getNodeDetails(node):
    details = {}

    # get node type
    details["Node Type"] = node["Node Type"]
    detailString = str(details["Node Type"])

    # get node conditions when needed
    values = getKeyValues(node, "cond")
    if len(values) > 0:
        details["Cond"] = values
        if type(details["Cond"]) != list:
            detailString += "\nby " + details["Cond"]
        else:
            detailString += "\nby " + ",".join(details["Cond"])

    # get filter information when needed
    if "Filter" in node:
        details["Filter"] = node["Filter"]
        if type(details["Filter"]) != list:
            detailString += "\nby " + details["Filter"]
        else:
            detailString += "\nby " + ",".join(details["Filter"])

    # get group key
    if "Group Key" in node:
        details["Group Key"] = node["Group Key"]
        if type(details["Group Key"]) != list:
            detailString += "\ngroup by " + details["Group Key"]
        else:
            detailString += "\ngroup by" + ",".join(details["Group Key"])

    # get sort key
    if "Sort Key" in node:
        details["Sort Key"] = node["Sort Key"]
        if type(details["Sort Key"]) != list:
            detailString += "\nsort by " + details["Sort Key"]
        else:
            detailString += "\nsort by " + ",".join(details["Sort Key"])

    # get method when necessary
    values = getKeyValues(node, "method")
    if len(values) > 0:
        details["Method"] = values
        if type(details["Method"]) == str:
            detailString += "\nusing " + details["Method"]
        else:
            detailString += "\nusing " + ",".join(details["Method"])

    # get relations
    if "Relation Name" in node:
        details["Relation Name"] = node["Relation Name"]
        if type(details["Relation Name"]) == str:
            detailString += "\non " + details["Relation Name"]
        else:
            detailString += "\non " + ",".join(details["Relation Name"])

    if "Alias" in node:
        details["Alias"] = node["Alias"]
        if type(details["Alias"]) is str:
            detailString += "\non " + details["Alias"]
        else:
            detailString += "\non " + ",".join(details["Alias"])

    # get output column details
    if "Output" in node:
        details["Output"] = node["Output"]
        if type(details["Output"]) == str:
            detailString += "\n" + details["Output"]
        else:
            detailString += "\n" + ",".join(details["Output"])

    # get string value to be shown on node.
    details["string"] = detailString
    return details


# def breakdown(curNode):
#     results = {
#         "node" : {},
#         "vertices" :  set(),
#         "edges" :  set(),
#         "sql" : {},
#     }
#     # get details of current node.
#     nodeDetails = getNodeDetails(curNode)
#     # print(nodeDetails)
#     results["node"][nodeDetails["string"]] = nodeDetails
#     results["vertices"].add(nodeDetails["string"])
#     results['sql'][nodeDetails["string"]] = getSQL(nodeDetails)
#
#     # break down child node
#     if "Plans" in curNode:
#         childSiblings = []
#         for node in curNode["Plans"]:
#             childNodeDetails = getNodeDetails(node)
#             results["edges"].add((nodeDetails["string"], childNodeDetails["string"]))
#             child_results = breakdown(node)
#             results["node"].update(child_results["node"])
#             results["vertices"].update(child_results["vertices"])
#             results["edges"].update(child_results["edges"])
#             results["sql"].update(child_results["sql"])
#             results['sql'][nodeDetails["string"]] = updateSQL(results['sql'][nodeDetails["string"]],
#                                                               results['sql'][childNodeDetails["string"]])
#             results["node"][childNodeDetails["string"]]['siblings'] = childSiblings
#             childSiblings.append(childNodeDetails["string"])
#
#     return results
#
#
# def getNodeDetails(node):
#     details = {"Node Type": node["Node Type"]}
#     detailString = str(details["Node Type"])
#
#     # Helper function to format and append details
#     def append_detail(key, prefix="", is_list_allowed=True, custom_join=","):
#         nonlocal detailString
#         if key in node:
#             value = node[key]
#             details[key] = value
#             if isinstance(value, list) and is_list_allowed:
#                 detailString += "\n" + prefix + custom_join.join(value)
#             else:
#                 detailString += "\n" + prefix + str(value)
#
#     # Append necessary details
#     append_detail("cond", "by ")
#     append_detail("Filter", "by ")
#     append_detail("Group Key", "group by ")
#     append_detail("Sort Key", "sort by ")
#     append_detail("method", "using ")
#     append_detail("Relation Name", "on ")
#     append_detail("Alias", "on ")
#     append_detail("Output")
#
#     details["string"] = detailString
#     return details


def concatRelationAlias(relation, alias):
    return relation if relation == alias else f"{relation} {alias}"
    # if relation == alias:
    #     return relation
    # else:
    #     return relation + " " + alias


# sub function used to get all values from specific keys
def getKeyValues(node_dict, search_key):
    result = []
    values = [value for key, value in node_dict.items() if search_key in key.lower()]
    for val in values:
        if type(val) == list:
            result.extend(val)
        else:
            result.append(val)
    return result


def concatenateRelation(a, b):
    return a if a is b else f"{a} {b}"


# a function to get details of node's sql query.
def getSQL(nodeDetails):
    sql = {}
    sql['select'] = []
    sql['relation'] = []
    sql['where'] = []
    sql['order by'] = []
    sql['group by'] = []
    if "Output" in nodeDetails:
        if type(nodeDetails['Output']) == list:
            sql['select'].extend(nodeDetails['Output'])
        else:
            sql['select'].append(nodeDetails['Output'])

    if "Relation Name" in nodeDetails:
        if type(nodeDetails['Relation Name']) == list:
            sql['relation'].extend(list(map(concatRelationAlias, nodeDetails['Relation Name'], nodeDetails['Alias'])))
        else:
            sql['relation'].append(concatRelationAlias(nodeDetails['Relation Name'], nodeDetails['Alias']))

    if "Sort Key" in nodeDetails:
        if type(nodeDetails['Sort Key']) == list:
            sql['order by'].extend(nodeDetails['Sort Key'])
        else:
            sql['order by'].append(nodeDetails['Sort Key'])
    if "Group Key" in nodeDetails:
        if type(nodeDetails['Group Key']) == list:
            sql['group by'].extend(nodeDetails['Group Key'])
        else:
            sql['group by'].append(nodeDetails['Group Key'])
    if "Cond" in nodeDetails:
        if type(nodeDetails['Cond']) == list:
            sql['where'].extend(nodeDetails['Cond'])
        else:
            sql['where'].append(nodeDetails['Cond'])

    if "Filter" in nodeDetails:
        if type(nodeDetails['Filter']) == list:
            sql['where'].extend(nodeDetails['Filter'])
        else:
            sql['where'].append(nodeDetails['Filter'])

    sql['where'] = list(set(sql['where']))
    return sql


# a function to update SQL details with child info node info
def updateSQL(nodeSQL, childNodeSQL):
    for value in childNodeSQL:
        if value == 'result' or value == 'select': continue
        nodeSQL[value].extend(childNodeSQL[value])
        nodeSQL[value] = list(set(nodeSQL[value]))  # remove duplicates
    return nodeSQL


# function to get query results from pieces of sql
def getQueryResults(sqlDetails, results, nodeName):
    # execute query and retrieve results.
    try:

        if 'Relation Name' in results['node'][nodeName]:
            print(results['node'][nodeName]['Node Type'])
            if type(results['node'][nodeName]['Alias']) is str:
                sqlDetails['select'].insert(0, f'{results["node"][nodeName]["Alias"]}.ctid')
            else:
                for relation in results['node'][nodeName]['Alias']:
                    sqlDetails['select'].insert(0, f'{relation}.ctid')
        sqlQuery = createQuery(sqlDetails)
        result = retrieveResult(sqlQuery, sqlDetails['select'])
        # print("pass1")
    except:  # error occurs if node requires "sibling" node information.

        for siblingNode in reversed(results['node'][nodeName]['siblings']):
            isUpdated = False
            relations = results["sql"][siblingNode]["relation"]
            for relation in relations:
                if relation in sqlDetails['relation']: continue
                for filter in sqlDetails['where']:
                    if relation in filter:
                        sqlDetails['relation'].extend(results["sql"][siblingNode]["relation"])
                        sqlDetails['relation'] = list(set(sqlDetails['relation']))
                        sqlDetails['where'].extend(results["sql"][siblingNode]["where"])
                        sqlDetails['where'] = list(set(sqlDetails['where']))
                        break
                if isUpdated: break
        sqlQuery = createQuery(sqlDetails)
        result = retrieveResult(sqlQuery, sqlDetails['select'])
        # print("pass2")
    # print(sqlQuery)

    if ('ctid' in " ".join(sqlDetails['select'])) and ("Index" in results['node'][nodeName]["Node Type"]):
        if 'Filter' in results['node'][nodeName]:
            if results['node'][nodeName]['Filter'] is not list:
                results['node'][nodeName]['Cond'] = [].append(results['node'][nodeName]['Filter'])
                sqlDetails['where'] = [cond for cond in sqlDetails['where'] if
                                       cond not in results['node'][nodeName]['Filter']]
                sqlQuery = createQuery(sqlDetails)
                accessResult = retrieveResult(sqlQuery, sqlDetails['select'])
            else:
                accessResult = None
    else:
        accessResult = None
    return result, accessResult


# a function to piece details of query together, return sql query
def createQuery(sqlDetails):
    # piece query
    sqlQuery = ""
    if len(sqlDetails['select']) > 0:
        sqlQuery += "select " + ",".join(sqlDetails['select'])
        sqlQuery = sqlQuery.replace("PARTIAL", "")
    if len(sqlDetails['relation']) > 0:
        sqlQuery += " from " + ",".join(sqlDetails['relation'])
    if len(sqlDetails['where']) > 0:
        sqlQuery += " where " + " AND ".join(sqlDetails['where'])
    if len(sqlDetails['group by']) > 0:
        sqlQuery += " group by " + ",".join(sqlDetails['group by'])
    if len(sqlDetails['order by']) > 0:
        sqlQuery += " order by " + ",".join(sqlDetails['order by'])
    return sqlQuery


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
    qep = retrieveQEP(sql)
    results = breakdown(qep["Plan"])
    results['final'] = retrieveResult(sql, qep["Plan"]["Output"])
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
from PyQt5 import QtCore, QtGui, QtQml
import numpy as np
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
    QApplication, QTableView, QVBoxLayout, QWidget, QGroupBox,
    QLabel, QScrollArea, QPushButton, QErrorMessage, QToolButton, QTabWidget
)
from PyQt5.QtCore import Qt, QPropertyAnimation, QEasingCurve, pyqtSignal
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
