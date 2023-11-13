# import libraries
import psycopg2
import pandas as pd
import networkx as nx
import pygraphviz as pgv

# define functions
def retrieveQEP(sql):
    conn = psycopg2.connect(database="SC3020Proj2",
                            host="localhost",
                            user="postgres",
                            password="password",
                            port="5432")

    cursor = conn.cursor()

    sql_exp = "explain (analyze, costs, verbose, buffers, format json) " + sql

    cursor.execute(sql_exp) 
    qep = cursor.fetchall() 

    conn.commit() 
    conn.close() 

    while(type(qep)!=dict):
        qep = qep[0]

    return qep

# break down current node and get node details.
def breakdown(curNode):
    results ={}
    results["node"] = {}
    results["vertices"] = set()
    results['edges'] = set()
    nodeDetails = getNodeDetails(curNode)
    results["node"][nodeDetails["string"]] = nodeDetails
    results["vertices"].add(nodeDetails["string"])

    # break down child node
    if "Plans" in curNode:
        for node in curNode["Plans"]:
            childNodeDetails = getNodeDetails(node)
            results["edges"].add((nodeDetails["string"],childNodeDetails["string"]))
            child_results = breakdown(node)
            results["node"].update(child_results["node"])
            results["vertices"].update(child_results["vertices"])
            results["edges"].update(child_results["edges"])
    
    return results

def getNodeDetails(node):
    details = {}
    details["Node Type"] = node["Node Type"]
    detailString = str(details["Node Type"])
    values = getKeyValues(node,"cond")
    if len(values) > 0:
        details["Cond"] = values
        if type(details["Cond"]) == str:
            detailString += "\nby "+details["Cond"]
        else: 
            detailString += "\nby "+",".join(details["Cond"])

    values = getKeyValues(node,"key")
    if len(values) > 0:
        details["Key"] = values
        if type(details["Key"]) == str:
            detailString += "\nby "+details["Key"]
        else: 
            detailString += "\nby "+",".join(details["Key"])

    values = getKeyValues(node,"method")
    if len(values) > 0:
        details["Method"] = values
        if type(details["Method"]) == str:
            detailString += "\nusing "+details["Method"]
        else: 
            detailString += "\nusing "+",".join(details["Method"])

    if "Output" in node: 
        details["Output"] = node["Output"]

    if "Relation Name" in node: 
        details["Relation Name"] = node["Relation Name"]
        if type(details["Relation Name"]) == str:
            detailString += "\n"+details["Relation Name"]
        else: 
            detailString += "\n"+",".join(details["Relation Name"])
    else: 
        if type(details["Output"]) == str:
            detailString += "\n"+details["Output"]
        else: 
            detailString += "\n"+",".join(details["Output"])

    details["string"] = detailString
    return details

def getKeyValues(node_dict,search_key):
    result = []
    values = [value for key, value in node_dict.items() if search_key in key.lower()]
    for val in values: 
        if type(val) == list:
            result.extend(val)
        else:
            result.append(val)
    return result

def plotQEP(results):
    G = pgv.AGraph(directed=True)
    G.add_nodes_from(results['vertices'])
    G.add_edges_from(results['edges'])

    G.write('test.dot')

    #create a png file
    G.layout(prog='dot') # use dot
    G.draw('file.png')

sql = '''
select
	l_returnflag,
	l_linestatus,
	sum(l_quantity) as sum_qty,
	sum(l_extendedprice) as sum_base_price,
	sum(l_extendedprice * (1 - l_discount)) as sum_disc_price,
	sum(l_extendedprice * (1 - l_discount) * (1 + l_tax)) as sum_charge,
	avg(l_quantity) as avg_qty,
	avg(l_extendedprice) as avg_price,
	avg(l_discount) as avg_disc,
	count(*) as count_order
from
	lineitem
where
	l_extendedprice < 33000
group by
	l_returnflag,
	l_linestatus
order by
	l_returnflag,
	l_linestatus;
'''
qep = retrieveQEP(sql)
results = breakdown(qep["Plan"])
plotQEP(results)




