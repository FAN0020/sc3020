# import libraries
import psycopg2
import pandas as pd
import networkx as nx
import pygraphviz as pgv

# define functions

# function to run sql to retrieve QEP of query. 
def retrieveQEP(sql):
    """function to run sql to retrieve QEP of query. 

    Args:
        sql (str): sql query to be executed

    Returns:
        json: detailed json format of the QEP 
    """
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


# 
def executeQuery(sql):
    """a function to connect to DB and execute query, returning results of query

    Args:
        sql (str): sql query to be executed

    Returns:
        _type_: result from executing the query
    """
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
            # add child information into final output.
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
    """a function to get the intermediary results at each node

    Args:
        results (dictionary): dictionary containing node's information, sql, and results.

    Returns:
        results: dictionary of all accumulated results
    """
    # get the intermediary results at each node
    for node in results['node']:
        result, accessResult = getQueryResults(results['sql'][node],results,node)
        for col in result.columns:
            if 'ctid' in col:
                result['block'] = result[col].apply(lambda x: int((x.replace("(","").replace(")","")).split(",")[0]))
                result['tuple'] = result[col].apply(lambda x: int((x.replace("(","").replace(")","")).split(",")[1]))
                results['sql'][node]['resultBlockAccess'] = result.groupby('block')['tuple'].apply(list)
                break

        results['sql'][node]["result"] = result

        # get information on the disks accessed
        print(type(accessResult))
        if type(accessResult) != type(None): # used for index scans where only specific blocks are accessed.
            for col in result.columns:
                if 'ctid' in col:
                    accessResult['block'] = accessResult[col].apply(lambda x: int((x.replace("(","").replace(")","")).split(",")[0]))
                    accessResult['tuple'] = accessResult[col].apply(lambda x: int((x.replace("(","").replace(")","")).split(",")[1]))
                    results['sql'][node]["accessResultBlockAccess"] = accessResult.groupby('block')['tuple'].apply(list)
                    break
        elif 'Relation Name' in results["node"][node]: # used for sequential scan (seq scan scans all, hence retrieve total number of blocks scanned)
            results['sql'][node]["RelationBlocks"] = executeQuery("SELECT relpages FROM pg_class WHERE relname = '" + results["node"][node]["Relation Name"]+"';")[0][0]
            
    return results



def getNodeDetails(node):
    """a function to retrieve required information from QEP nodes

    Args:
        node (dictionary): detailed information of a node provided from the QEP retrieved

    Returns:
        dictionary: dictionary of important node information needed for exploring the QEP
    """
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


def concatRelationAlias(relation, alias):
    """a function to concatenate the relation and alias for use in the "from" portion of the relation.

    Args:
        relation (str): relation name
        alias (str): alias used to represent the relation name

    Returns:
        str: relation name if there is no alias, else relation + alias.
    """
    if relation == alias:
        return relation
    else:
        return relation + " " + alias


def getKeyValues(node_dict,search_key):
    """a sub function used to get all values from specific keys

    Args:
        node_dict (dictionary): dictionary containing node details
        search_key (_type_): the substring of a key we're looking for

    Returns:
        list: a list of all the values from keys containing search_key
    """
    result = []
    values = [value for key, value in node_dict.items() if search_key in key.lower()]
    for val in values: 
        if type(val) == list:
            result.extend(val)
        else:
            result.append(val)
    return result


# a function to get details of node's sql query.
def getSQL(nodeDetails):
    """a function to get details of node's sql query

    Args:
        nodeDetails (dictionary): dictionary containing node details

    Returns:
        dictionary: a dictionary of all the pieces of an sql
    """
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
def updateSQL(nodeSQL,childNodeSQL):
    """a function to update SQL details with child node info as parent node would use the same filters as child node.

    Args:
        nodeSQL (dictionary): dictionary of sql pieces of the current node.
        childNodeSQL (dictionary): dictionary of sql pieces of the child node.

    Returns:
        dictionary: updated sql dictionary of the current node
    """
    for value in childNodeSQL:
        if value == 'result' or value == 'select': continue
        nodeSQL[value].extend(childNodeSQL[value])
        nodeSQL[value] = list(set(nodeSQL[value])) # remove duplicates
    return nodeSQL

# function to get query results from pieces of sql
def getQueryResults(sqlDetails, results, nodeName):
    """a function to get query results from pieces of sql

    Args:
        sqlDetails (dictionary): dictionary of sql details of the node.
        results (dictionary): dictionary containing accumulated results
        nodeName (str): name of the current node

    Returns:
        result (dataframe): dataframe containing the intermediary results of node.
        accessResult (dictionary): dataframe containing the details of the blocks accessed and tuples accessed.
    """
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
        #print("pass1")
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
        #print("pass2")
    #print(sqlQuery)
    
    if ('ctid' in " ".join(sqlDetails['select'])) and  ("Index" in results['node'][nodeName]["Node Type"]):
        if 'Filter' in results['node'][nodeName]:
            if results['node'][nodeName]['Filter'] is not list:
                results['node'][nodeName]['Cond'] = [].append(results['node'][nodeName]['Filter'])
                sqlDetails['where'] = [cond for cond in sqlDetails['where'] if cond not in results['node'][nodeName]['Filter']]
                sqlQuery = createQuery(sqlDetails)
                accessResult = retrieveResult(sqlQuery, sqlDetails['select'])
            else: accessResult = None
    else: accessResult = None
    return result, accessResult

# a function to piece details of query together, return sql query
def createQuery(sqlDetails):
    """a function to piece the details of an sql query together.

    Args:
        sqlDetails (dictionary ): broken down details of an sql query

    Returns:
        str: pieced sql query
    """
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
qep = retrieveQEP(sql)
results = breakdown(qep["Plan"])
results['final'] = retrieveResult(sql,qep["Plan"]["Output"])
plotQEP(results)

results = getIntermediateResults(results)
        

    




