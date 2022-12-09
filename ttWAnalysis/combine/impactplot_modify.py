#####################################################
# modify a json object used for making impact plots #
#####################################################
# usage: 
# the need for this script started from the observation that some impact plots yielded
# many strongly asymmetric or even one-sided impacts.
# it seems to be an effect of the initial fit that is off somehow;
# if replacing the best-fit value in the json with the one obtained from e.g. FitDiagnostics,
# the impacts come out much better...
# still to discuss if this is 'clean' enough...

import sys
import os
import json
import outputparsetools as opt
sys.path.append('../tools')
import listtools as lt

if __name__=='__main__':

    if len(sys.argv)==2:
	inputfile = sys.argv[1]
    else:
	raise Exception('ERROR: wrong number of command line arguments')

    # check if argument is a datacard or workspace
    datacarddir = os.path.dirname(inputfile)
    inputname = ''
    if inputfile[-5:]=='.root': inputname = inputfile.split('/')[-1].replace('.root','')
    elif inputfile[-4:]=='.txt': inputname = inputfile.split('/')[-1].replace('.txt','')
    # extension: if folder, run on all datacards in it
    elif '.' not in inputfile:
	datacards = [f for f in os.listdir(inputfile) if '.txt' in f]
	datacards = lt.subselect_strings(datacards,mustcontainone=['datacard','dc_combined'],
			    maynotcontainone=['_out_','controlregion','signalsideband'])[1]
	datacards = [os.path.join(inputfile,f) for f in datacards]
	for d in datacards: 
	    cmd = 'python impactplot_modify.py '+d
	    os.system(cmd)
    else: raise Exception('ERROR: input file must be a datacard or associated root workspace')

    # check if fit result exists and read best fit value
    fitresultfile = os.path.join(datacarddir,inputname+'_out_fitdiagnostics_obs.txt')
    if not os.path.exists(fitresultfile):
	raise Exception('ERROR: fit result file '+fitresultfile+' does not seem to exist...')
    (newbestfit,downerror,uperror) = opt.read_r( datacarddir, inputname+'.txt', usedata=True )
    print('found new best-fit value {}'.format(newbestfit))
    
    # check if impact json exists and read it
    jsonfile = os.path.join(datacarddir,inputname+'_obs',
			    inputname+'_obs_impacts.json')
    if not os.path.exists(jsonfile):
        raise Exception('ERROR: impact json file '+jsonfile+' does not seem to exist...')
    jsonobj = None
    with open(jsonfile) as f:
	jsonobj = json.load(f)
    poi = jsonobj['POIs']
    if len(poi)!=1:
	raise Exception('ERROR: found unexpected number of POIs: {}'.format(len(poi)))
    poi = poi[0]
    print('replacing old best-fit value {}'.format(poi['fit'][1]))

    # change the best-fit (central) value for the header
    poi['fit'][1] = newbestfit
    poi['fit'][0] = newbestfit-downerror
    poi['fit'][2] = newbestfit+uperror

    # change the best-fit (central) value for all params
    # also change the impact
    for param in jsonobj['params']:
	param['r'][1] = newbestfit
	param['impact_r'] = max(abs(param['r'][0]-newbestfit),abs(param['r'][2]-newbestfit))

    # write the modified json
    outputfile = jsonfile.replace('.json','_mod.json')
    with open(outputfile,'w') as f:
	json.dump(jsonobj,f)

    # make the modified impact plot
    fig = outputfile.replace('.json','')
    cmd = 'plotImpacts.py -i {} -o {}'.format(outputfile,fig)
    os.system(cmd)
