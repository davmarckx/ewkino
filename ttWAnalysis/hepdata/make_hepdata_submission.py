###################################################
# script for generating a HEPData submission file #
###################################################
# based on TZQ
# references:
# - https://www.hepdata.net/submission
# - https://github.com/HEPData/hepdata_lib/
# note: requires the third-party python module "hepdata_lib"!
#       can be installed simply via "pip install hepdata_lib"
#       (preferably in a virtual environment)
#       tested in CMSSW_12_4_6 and python3 venv
#
# TODO: add the locations of this data in the paper

import hepdata_lib
from hepdata_lib import Submission, Table, RootFileReader, Variable, Uncertainty
import os
import sys
sys.path.append(os.path.abspath('../../Tools/python'))
from variabletools import read_variables

from parse_th2 import read_hist_2d_labeled
from read_txt_file import readchanneltxt
from read_json_file import read_impact_json

def get_all_signalregions(splits=False):
    regions = []
    for r in ['dilepton_inclusive','trilepton']: regions.append('signalregion_{}'.format(r))
    if splits: 
        for r in ['ee','em','me','mm','plus','minus']: regions.append('signalregion_dilepton_{}'.format(r))
    return regions

def get_all_controlregions():
    regions = ['wzcontrolregion','zzcontrolregion','zgcontrolregion']
    for r in ['trileptoncontrolregion','fourleptoncontrolregion']: regions.append(r)
    for r in ['npcontrolregion_dilepton_inclusive','cfcontrolregion']: regions.append(r)
    return regions

def get_region_description( region ):
    if region=='signalregion_dilepton_inclusive': return 'the signal region with exactly 2 SS leptons'
    elif region=='signalregion_trilepton': return 'the signal region with exactly 3 leptons'
    elif region=='signalregion_dilepton_plus': return 'the signal region with two positive sign leptons'
    elif region=='signalregion_dilepton_minus': return 'the signal region with two negative sign leptons'
    elif region=='signalregion_dilepton_ee': return 'the signal region with two SS electrons'
    elif region=='signalregion_dilepton_em': return 'the signal region with a leading electron and subleading SS muon'
    elif region=='signalregion_dilepton_me': return 'the signal region with a leading muon and subleading SS electron'
    elif region=='signalregion_dilepton_mm': return 'the signal region with two SS muons'

    
    else:
        raise Exception('ERROR in get_region_description: region {} not recognized'.format(region))



def get_all_regions(splits=False):
    
    regions = get_all_signalregions(splits)
    regions += get_all_controlregions()
    
    return regions

def get_all_processes(np_origin):
    if np_origin == 'datasplit':
        processes = ["TTZ", "TTX", "TX", "Multiboson", "TT", "ZG", "TTW", "Nonprompt(e)", "ZZ", "DY", "Chargeflips","TTG","WZ","Nonprompt(m)"]
    elif np_origin == 'data':
        processes = ["TTZ", "TTX", "TX", "Multiboson", "TT", "ZG", "TTW", "Nonprompt", "ZZ", "DY", "Chargeflips","TTG","WZ"]
    else:
        raise Exception('ERROR in get_all_processes: np origin {} not recognized'.format(np_origin))
    return processes





if __name__=='__main__':

    ### global settings
    year = "run2"
    theoryyear = "2018" #year from which theory is taken (shouldn't matter)
    np_origin = 'datasplit'
    cf_origin = 'data'
    # where you take np and cf predictions from

    directory_input = 'input' #dont change!!!
    # (where to copy input files to and read them from for making the submission)

    directory_input_raw = 'input_raw'
    # (where hardcoded inputs are stored, e.g. abstract.txt)

    directory_input_diffplots = "/user/llambrec/public/output_20230718/"
    directory_input_diffhists = "~/ewkino/ttWAnalysis/differential/output_test/" # will become directory_input_diffplots + "/rootfiles"
    diff_variables = "../variables/variables_particlelevel_single.json"
    # (where differential histograms and figures are stored)

    directory_output = 'output' 
    # (where to write output files)

    ### copy files to input directory
    
    print('copying input files...')

    sr_map = {'signalregion_dilepton_inclusive':'dilepton SR', 'signalregion_trilepton':'trilepton SR'} 

    # clear directory
    if os.path.exists(directory_input):
        os.system('rm -r {}'.format(directory_input))
    os.makedirs(directory_input)

    # copy differential root files
    for sr in get_all_signalregions(splits=False):
      infile = directory_input_diffhists
      infile += "{}/rootfiles/differentialplots_{}.root".format(sr,year)
      outfile = os.path.join(directory_input, "differentialplots_{}_{}.root".format(sr,year))
      os.system('cp {} {}'.format(infile,outfile))

    # copy differential figures
    diffvarlist = [x.name for x in read_variables( diff_variables )]
    diffvar_shortnamelist = [x.shorttitle for x in read_variables( diff_variables )]
    print(diffvarlist)
    for fit in ['nocr','withcr']:
        for sr in get_all_signalregions(splits=False):
           for diffvar in diffvarlist: 
               infile = directory_input_diffplots
               norminfile = infile + "/{}/particlelevel/plots_obs_{}/{}_norm.png".format(sr,fit,diffvar)
               infile += "/{}/particlelevel/plots_obs_{}/{}.png".format(sr,fit,diffvar)
               
               outfile = os.path.join(directory_input, "{}_{}_{}.png".format(sr,fit,diffvar))
               normoutfile = os.path.join(directory_input, "{}_{}_{}_norm.png".format(sr,fit,diffvar))
               os.system('cp {} {}'.format(infile,outfile))
               os.system('cp {} {}'.format(norminfile,normoutfile))




    ### create submission

    print('making initial submission object...')
    submission = Submission()
    submission.read_abstract(os.path.join(directory_input_raw,'abstract.txt'))

    ### add differential results for each variable
    fit = "withcr"
    signalregions = ["signalregion_dilepton_inclusive","signalregion_trilepton"]
    for sr in signalregions:
        for i,var in enumerate(diffvarlist):
          print('adding differential results for {} in {}'.format(var,sr_map[sr]))    
          table = Table('Diff. results for {} in {}'.format(diffvar_shortnamelist[i], sr_map[sr]))
          descr = 'Numerical results of differential cross section measurements for {}.'.format(var)
          table.description = descr

          normtable = Table('Norm. diff. results for {} in {}'.format(diffvar_shortnamelist[i], sr_map[sr]))
          descr = 'Numerical results of normalized differential cross section measurements for {}.'.format(var)
          normtable.description = descr

          reader = RootFileReader(os.path.join(directory_input,'differentialplots_{}_{}.root'.format(sr,year)))

          
          datahist = reader.read_hist_1d('TTW{}_{}_particlelevel_{}_data'.format(theoryyear,sr,var))
          datastathist = reader.read_hist_1d('TTW{}_{}_particlelevel_{}_datastat'.format(theoryyear,sr,var))
          theoryhist = reader.read_hist_1d('TTW{}_{}_particlelevel_{}_MC'.format(theoryyear,sr,var))
          theorysysthist = reader.read_hist_1d('TTW{}_{}_particlelevel_{}_MC_syst'.format(theoryyear,sr,var))

          normdatahist = reader.read_hist_1d('TTW{}_{}_particlelevel_{}_normdata'.format(theoryyear,sr,var))
          normdatastathist = reader.read_hist_1d('TTW{}_{}_particlelevel_{}_normstatdata'.format(theoryyear,sr,var))
          normtheoryhist = reader.read_hist_1d('TTW{}_{}_particlelevel_{}_normMC'.format(theoryyear,sr,var))
          normtheorysysthist = reader.read_hist_1d('TTW{}_{}_particlelevel_{}_normMC_syst'.format(theoryyear,sr,var))

          # create variables
          variable = Variable(var, is_independent=True, is_binned=False)
          variable.values = datahist['x']

          data = Variable('Observed differential ttW cross section', is_independent=False, is_binned=False)
          data.values = datahist['y']
          data_unc = Uncertainty('Total uncertainty' )
          data_unc.values = datahist['dy']
          data.add_uncertainty( data_unc )
          data_unc_stat = Uncertainty('Statistical uncertainty')
          data_unc_stat.values = datastathist['dy']
          data.add_uncertainty( data_unc_stat )


          theory = Variable('Expected differential ttW cross section', is_independent=False, is_binned=False)
          theory.values = theoryhist['y']
          theory_unc_syst = Uncertainty('Systematic uncertainty')
          theory_unc_syst.values = theorysysthist['dy']
          theory.add_uncertainty( theory_unc_syst )


          normdata = Variable('Observed normalized differential ttW cross section', is_independent=False, is_binned=False)
          normdata.values = normdatahist['y']
          normdata_unc = Uncertainty('Total uncertainty' )
          normdata_unc.values = normdatahist['dy']
          normdata.add_uncertainty( normdata_unc )
          normdata_unc_stat = Uncertainty('Statistical uncertainty')
          normdata_unc_stat.values = normdatastathist['dy']
          normdata.add_uncertainty( normdata_unc_stat )


          normtheory = Variable('Expected normalized differential ttW cross section', is_independent=False, is_binned=False)
          normtheory.values = normtheoryhist['y']
          normtheory_unc_syst = Uncertainty('Systematic uncertainty')
          normtheory_unc_syst.values = normtheorysysthist['dy']
          normtheory.add_uncertainty( normtheory_unc_syst )


          # add variables to table
          table.add_variable(variable)
          table.add_variable(data)
          table.add_variable(theory)
          table.add_image(directory_input + "/{}_{}_{}.png".format(sr,fit,var,year))


          normtable.add_variable(variable)
          normtable.add_variable(normdata)
          normtable.add_variable(normtheory)
          normtable.add_image(directory_input + "/{}_{}_{}_norm.png".format(sr,fit,var,year))

        
          # add table to submission
          submission.add_table(table)
          submission.add_table(normtable)


    ### create the output files
    print('writing output files...')
    if os.path.exists(directory_output):
        os.system('rm -r {}'.format(directory_output))
    os.makedirs(directory_output)
    submission.create_files(directory_output)
