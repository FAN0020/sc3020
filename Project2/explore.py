# import libraries
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
    while(type(qep)!=dict):
        qep = qep[0]

    return qep

# function to run sql query and rerieve results (dataframe)
def retrieveResult(sql,columns):
    # execute query
    result = executeQuery(sql)
    df = pd.DataFrame(result,columns=columns)
    return df


# a function to connect to DB and execute query, returning results of query
def executeQuery(sql):
    # connect to database. 
    # edit values according to your own database.
    conn = psycopg2.connect(database="SC3020Proj2",
                            host="localhost",
                            user="postgres",
                            password="password",
                            port="5432")

    cursor = conn.cursor()

    # run query and retrieve results
    cursor.execute(sql) 
    result = cursor.fetchall() 

    conn.commit() 
    conn.close() 

    #return results
    return result

# break down current node and get node details.
def breakdown(curNode):
    results ={}
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
            results["edges"].add((nodeDetails["string"],childNodeDetails["string"]))
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

def getIntermediateResults(results):
    for node in results['node']:
        results['sql'][node]["result"] = getQueryResults(results['sql'][node],results,node)
    return results


# a function get details of a node.
def getNodeDetails(node):
    details = {}

    # get node type
    details["Node Type"] = node["Node Type"]
    detailString = str(details["Node Type"])

    # get node conditions when needed
    values = getKeyValues(node,"cond")
    if len(values) > 0:
        details["Cond"] = values
        if type(details["Cond"]) != list:
            detailString += "\nby "+details["Cond"]
        else: 
            detailString += "\nby "+",".join(details["Cond"])
    
    # get filter information when needed
    if "Filter" in node: 
        details["Filter"] = node["Filter"]
        if type(details["Filter"]) != list:
            detailString += "\nby "+details["Filter"]
        else: 
            detailString += "\nby "+",".join(details["Filter"])

    # get group key
    if "Group Key" in node: 
        details["Group Key"] = node["Group Key"]
        if type(details["Group Key"]) != list:
            detailString += "\ngroup by "+details["Group Key"]
        else: 
            detailString += "\ngroup by"+",".join(details["Group Key"])
    
    # get sort key
    if "Sort Key" in node: 
        details["Sort Key"] = node["Sort Key"]
        if type(details["Sort Key"]) != list:
            detailString += "\nsort by "+details["Sort Key"]
        else: 
            detailString += "\nsort by "+",".join(details["Sort Key"])

    # get method when necessary
    values = getKeyValues(node,"method")
    if len(values) > 0:
        details["Method"] = values
        if type(details["Method"]) == str:
            detailString += "\nusing "+details["Method"]
        else: 
            detailString += "\nusing "+",".join(details["Method"])

    # get relations
    if "Relation Name" in node: 
        details["Relation Name"] = node["Relation Name"]
        if type(details["Relation Name"]) == str:
            detailString += "\non "+details["Relation Name"]
        else: 
            detailString += "\non "+",".join(details["Relation Name"])
    
    # get output column details
    if "Output" in node: 
        details["Output"] = node["Output"]
        if type(details["Output"]) == str:
            detailString += "\n"+details["Output"]
        else: 
            detailString += "\n"+",".join(details["Output"])

    
    # get string value to be shown on node.
    details["string"] = detailString
    return details

# sub function used to get all values from specific keys
def getKeyValues(node_dict,search_key):
    result = []
    values = [value for key, value in node_dict.items() if search_key in key.lower()]
    for val in values: 
        if type(val) == list:
            result.extend(val)
        else:
            result.append(val)
    # for i in range(len(result)):
    #     print(result[i])
    #     print(type(result[i]))
    return result

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
            sql['relation'].extend(nodeDetails['Relation Name'])
        else: 
            sql['relation'].append(nodeDetails['Relation Name'])
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
def updateSQL(nodeSQL,childNodeSQL):
    for value in childNodeSQL:
        if value == 'result' or value == 'select': continue
        nodeSQL[value].extend(childNodeSQL[value])
        nodeSQL[value] = list(set(nodeSQL[value])) # remove duplicates
    return nodeSQL

# function to get query results from pieces of sql
def getQueryResults(sqlDetails,results,nodeName):
    
    # execute query and retrieve results.
    try:
        sqlQuery = createQuery(sqlDetails)
        result = retrieveResult(sqlQuery,sqlDetails['select'])
        print("pass1")
    except: # error occurs if node requires "sibling" node information.
        isUpdated = False
        for siblingNode in reversed(results['node'][nodeName]['siblings']):
            relations = results["sql"][siblingNode]["relation"]
            for relation in relations:
                if relation in sqlDetails['relation']: continue
                for filter in sqlDetails['where']:
                    if relation in filter: 
                        sqlDetails['relation'].extend(results["sql"][siblingNode]["relation"])
                        sqlDetails['relation'] = list(set(sqlDetails['relation']))
                        sqlDetails['where'].extend(results["sql"][siblingNode]["where"])
                        sqlDetails['where'] = list(set(sqlDetails['where']))
                        sqlQuery = createQuery(sqlDetails)
                        result = retrieveResult(sqlQuery,sqlDetails['select'])
                        break
                if isUpdated: break
            if isUpdated: break
        print("pass2")

    return result

def getDiskAccessDetails(results):
    for node in results['node']:
        if "Relation Name" in results['node'][node]:
            sqlDetails = results['sql'][node]
            try:
                sqlDetails['select'].insert(0,'ctid')
                sqlQuery = createQuery(sqlDetails)
                result = retrieveResult(sqlQuery,sqlDetails['select'])
                print("pass1")
            except: # error occurs if node requires "sibling" node information.
                isUpdated = False
                for siblingNode in reversed(results['node'][node]['siblings']):
                    relations = results["sql"][siblingNode]["relation"]
                    for relation in relations:
                        if relation in sqlDetails['relation']: continue
                        for filter in sqlDetails['where']:
                            if relation in filter: 
                                sqlDetails['relation'].extend(results["sql"][siblingNode]["relation"])
                                sqlDetails['relation'] = list(set(sqlDetails['relation']))
                                sqlDetails['where'].extend(results["sql"][siblingNode]["where"])
                                sqlDetails['where'] = list(set(sqlDetails['where']))
                                sqlDetails['select'].insert(0,'ctid')
                                sqlQuery = createQuery(sqlDetails)
                                result = retrieveResult(sqlQuery,sqlDetails['select'])
                                break
                        if isUpdated: break
                    if isUpdated: break
                print("pass2")  
            results['sql'][node]['ctid'] = result
    return results
              

# a function to piece details of query together, return sql query
def createQuery(sqlDetails):
    # piece query
    sqlQuery = ""
    if len(sqlDetails['select']) > 0: 
        sqlQuery += "select " + ",".join(sqlDetails['select'])
        sqlQuery = sqlQuery.replace("PARTIAL","")
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

# print(results)

results = getDiskAccessDetails(results)
results = getIntermediateResults(results)

for node in results['sql']:
    if "Relation Name" in results['node'][node]:
        print(results['sql'][node]['ctid'])

        

    




