#########################################
# help functions for reading json files #
#########################################
# COPIED FROM TZQ
# usage: impact json files

import json

def read_impact_json( filename_obs, filename_exp ):
    # copied partly from customImpacts.py!

    data = {}
    with open(filename_obs) as jsonfile:
        data = json.load(jsonfile)

    asidata = {}
    with open(filename_exp) as asifile:
        asidata = json.load(asifile)

    POIs = [ele['name'] for ele in data['POIs']]
    POI = POIs[0]

    for ele in data['POIs']:
        if ele['name'] == POI:
                POI_info = ele
                break

    POI_fit = POI_info['fit']

    # sort data by impact
    data['params'].sort(key=lambda x: abs(x['impact_%s' % POI]), reverse=True)
    # sort asimov data corresponding to data
    asidatasorted = []
    for i,el in enumerate(data['params']):
        thisname = el['name']
        asipnum = -1
        asipnumfound = False
        while not asipnumfound:
            asipnum +=1
            if asidata['params'][asipnum]["name"]==thisname:
                asipnumfound = True
            elif asipnum==len(asidata['params'])-1:
                # option 1: just break the loop and take last one in list
                break
                # option 2: throw exception
                #raise Exception('ERROR in read_impact_json: parameter {}'.format(thisname)
                #               +' has no counterpart in expected results!')
        asidatasorted.append(asidata['params'][asipnum])

    return (data['params'],asidatasorted)
