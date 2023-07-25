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
# tested in CMSSW_12_4_6 and python3

import hepdata_lib
from hepdata_lib import Submission, Table, RootFileReader, Variable, Uncertainty
import os

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
    elif region=='signalregion_dilepton_ee': return 'the signal region with two electrons'
    elif region=='signalregion_dilepton_em': return 'the signal region with a leading electron and subleading muon'
    elif region=='signalregion_dilepton_me': return 'the signal region with a leading muon and subleading electron'
    elif region=='signalregion_dilepton_mm': return 'the signal region with two muons'

    
    else:
        raise Exception('ERROR in get_region_description: category {} not recognized'.format(cat))

def get_all_regions(splits=False):
    
    regions = get_all_signalregions(splits)
    regions += get_all_controlregions()
    
    return regions

def get_all_processes(np_origin):
    if np_origin == 'datasplit':
        processes = ["TTZ", "TTX", "TX", "Multiboson", "TT", "ZG", "TTW", "Nonprompt(e)", "ZZ", "DY", "Chargeflips","TTG","WZ","Nonprompt(m)"]
    else if np_origin == 'data':
        processes = ["TTZ", "TTX", "TX", "Multiboson", "TT", "ZG", "TTW", "Nonprompt", "ZZ", "DY", "Chargeflips","TTG","WZ"]
    else:
        raise Exception('ERROR in get_all_processes: np origin {} not recognized'.format(np_origin))
    return processes

if __name__=='__main__':

    ### global settings

    np_origin = 'datasplit'
    cf_origin 'data'
    # where you take np and cf predictions from

    directory_input = 'input' 
    # (where to copy input files to and read them from for making the submission)
    directory_input_raw = 'input_raw'
    # (where hardcoded inputs are stored, e.g. abstract.txt)
    directory_input_hists = 'input_hists'
    # (where histograms and figures are stored)
    directory_output = 'output' 
    # (where to write output files)

    ### copy files to input directory
    
    print('copying input files...')

    # clear directory
    if os.path.exists(directory_input):
        os.system('rm -r {}'.format(directory_input))
    os.makedirs(directory_input)

    # copy combine fit result file for totally inclusive measurement                                                                #to be updated!!!
    infile = os.path.join(directory_input_hists,'covariance',
        'fitDiagnosticsdc_combined_all_out_fitdiagnostics_obs.root' )
    outfile = os.path.join(directory_input,'fitDiagnostics_inclusive.root')
    os.system('cp {} {}'.format(infile,outfile))
    
    # copy postFitShapesFromWorkspace files for totally inclusive measurements                                                      #to be updated!!!
    for fit in ['prefit','postfit']:
        for sr in get_all_signalregions(splits=True):
            infile = directory_input_hists
            infile += '/{}/run2/signalregion_{}/'.format(fit,cat)
            infile += 'dc_combined_signalregion_{}_yearscombined'.format(cat)
            infile += '_var__eventBDT_temp.root'
            outfile = os.path.join(directory_input,'signalregion_{}_{}.root'.format(cat,fit))
            os.system('cp {} {}'.format(infile,outfile))
            infile = directory_input_hists
            infile += '/{}/signalregion_{}_yearscombined/'.format(fit,cat)
            infile += 'dc_combined_signalregion_{}_yearscombined'.format(cat)
            infile += '_var__eventBDT_statonly_temp.root'
            outfile = os.path.join(directory_input,'signalregion_{}_{}_stat.root'.format(cat,fit))
            os.system('cp {} {}'.format(infile,outfile))

    # copy figures                                                                                                                  #to be updated!!!
    for fit in ['prefit','postfit']:                                                                                       
        for sr in get_all_signalregions(splits=True):
            infile = directory_input_hists
            infile += '/output_{}/run2/{}/'.format(fit,sr)
            infile += 'merged_npfrom{}_cffrom{}/plots/_eventBDT.pdf'.format(np_origin,cf_origin)
            outfile = os.path.join(directory_input,'signalregion_{}_{}.pdf'.format(cat,fit))
            os.system('cp {} {}'.format(infile,outfile))

    ### create submission

    print('making initial submission object...')
    submission = Submission()
    submission.read_abstract(os.path.join(directory_input_raw,'abstract.txt'))

    ### add the numerical results of inclusive measurements
    
    print('adding numerical results...')
    channels = readchanneltxt(os.path.join(directory_input_raw,'results.txt'))
    table = Table('results')
    descr = 'Numerical results of inclusive cross section measurements.'
    descr += ' Each row represents a measurement: "ttW" for fully inclusive,'
    descr += ' "ttW_+" for the positive dilepton channel,'
    descr += ' "ttW_-" for the negative dilepton channel,'
    descr += ' "ratio" for the ratio measurement.'
    descr += ' The columns are the central value, statistical error up/down,'
    descr += ' systematic error up/down.'
    descr += ' All values are in fb, except for the ratio (dimensionless).'
    table.description = descr
    table.location = 'This table summarizes the main results given in the text of the paper'
    channelnames = Variable('channel name', is_independent=True, is_binned=False)
    channelnames.values = [channel[0] for channel in channels]
    central = Variable('cross section', is_independent=False, is_binned=False)
    central.values = [channel[2] for channel in channels]
    statup = Variable('stat. error up', is_independent=False, is_binned=False)
    statup.values = [channel[3] for channel in channels]
    statdown = Variable('stat. error down', is_independent=False, is_binned=False)
    statdown.values = [channel[4] for channel in channels]
    systup = Variable('syst. error up', is_independent=False, is_binned=False)
    systup.values = [channel[5] for channel in channels]
    systdown = Variable('syst. error down', is_independent=False, is_binned=False)
    systdown.values = [channel[6] for channel in channels]
    table.add_variable(channelnames)
    table.add_variable(central)
    table.add_variable(statup)
    table.add_variable(statdown)
    table.add_variable(systup)
    table.add_variable(systdown)
    submission.add_table(table)

    ### add the numerical results of impact plot
    print('adding impacts...')
    (impact_data_obs, impact_data_exp) = read_impact_json(
                    os.path.join(directory_input_raw,'impacts_observed.json'),
                    os.path.join(directory_input_raw,'impacts_expected.json'))
    impact_data_obs = impact_data_obs[:8]
    impact_data_exp = impact_data_exp[:8]
    table = Table('impacts')
    descr = 'Numerical representation of impact plot.'
    table.description = descr
    table.location = 'This table is a numerical representation of Fig. 8.'
    impact_names = Variable('impact_names', is_independent=True, is_binned=False)
    impact_names.values = [str(el['name']) for el in impact_data_obs]
    fit_key = 'fit'
    fits_obs_down = Variable('fit_obs_down', is_independent=False, is_binned=False)
    fits_obs_down.values = [el[fit_key][0] for el in impact_data_obs]
    fits_obs_central = Variable('fit_obs_central', is_independent=False, is_binned=False)
    fits_obs_central.values = [el[fit_key][1] for el in impact_data_obs]
    fits_obs_up = Variable('fits_obs_up', is_independent=False, is_binned=False)
    fits_obs_up.values = [el[fit_key][2] for el in impact_data_obs]
    fits_exp_down = Variable('fits_exp_down', is_independent=False, is_binned=False)
    fits_exp_down.values = [el[fit_key][0] for el in impact_data_exp]
    fits_exp_central = Variable('fits_exp_central', is_independent=False, is_binned=False)
    fits_exp_central.values = [el[fit_key][1] for el in impact_data_exp]
    fits_exp_up = Variable('fits_exp_up', is_independent=False, is_binned=False)
    fits_exp_up.values = [el[fit_key][2] for el in impact_data_exp]
    poi_key = 'mu'
    impacts_obs_down = Variable('impacts_obs_down', is_independent=False, is_binned=False)
    impacts_obs_down.values = [el[poi_key][0] for el in impact_data_obs]
    impacts_obs_central = Variable('impacts_obs_central', is_independent=False, is_binned=False)
    impacts_obs_central.values = [el[poi_key][1] for el in impact_data_obs]
    impacts_obs_up = Variable('impacts_obs_up', is_independent=False, is_binned=False)
    impacts_obs_up.values = [el[poi_key][2] for el in impact_data_obs]
    impacts_exp_down = Variable('impacts_exp_down', is_independent=False, is_binned=False)
    impacts_exp_down.values = [el[poi_key][0] for el in impact_data_exp]
    impacts_exp_central = Variable('impacts_exp_central', is_independent=False, is_binned=False)
    impacts_exp_central.values = [el[poi_key][1] for el in impact_data_exp]
    impacts_exp_up = Variable('impacts_exp_up', is_independent=False, is_binned=False)
    impacts_exp_up.values = [el[poi_key][2] for el in impact_data_exp]
    table.add_variable(impact_names)
    table.add_variable(fits_obs_down)
    table.add_variable(fits_obs_central)
    table.add_variable(fits_obs_up)
    table.add_variable(fits_exp_down)
    table.add_variable(fits_exp_central)
    table.add_variable(fits_exp_up)
    table.add_variable(impacts_obs_down)
    table.add_variable(impacts_obs_central)
    table.add_variable(impacts_obs_up)
    table.add_variable(impacts_exp_down)
    table.add_variable(impacts_exp_central)
    table.add_variable(impacts_exp_up)
    submission.add_table(table)

    ### make Tables for totally inclusive measurement

    print('making tables...')

    for fit in ['prefit','postfit']:
        for sr in get_all_signalregions():

            # meta-information
            table = Table('table_{}'.format(sr,fit))
            descr = 'Simulated signal, total background, and observed data'
            descr += ' in {}'.format(get_region_description(sr))
            descr += ' for the three data-taking years combined.'
            descr += ' For the uncertainty on the signal and background,'
            descr += ' both the total (systematic+statistical)'
            descr += ' and statistical uncertainties are provided.'
            descr += ' The uncertainty on the data'
            descr += ' is the (statistical) Poisson uncertainty.'
            descr += ' Note that this is the {} version.'.format(fit)
            table.description = descr
            cat_to_loc = {'signalregion_dilepton_inclusive':'upper','signalregion_trilepton':'lower'}
            fit_to_loc = {'prefit':'left','postfit':'right'}
            table.location = 'Figure 7 ({}, {})'.format(sr_to_loc[sr],fit_to_loc[fit])
            # line below does not seem to work since ImageMagick is probably not installed on T2
            #table.add_image(os.path.join(directory_input,'signalregion_{}.pdf'.format(cat)))
        
            # read the histograms
            reader = RootFileReader(os.path.join(directory_input,
                                'signalregion_{}_{}.root'.format(cat,fit)))
            tzqhist = reader.read_hist_1d('{}/{}'.format(fit,'TotalSig'))
            bckhist = reader.read_hist_1d('{}/{}'.format(fit,'TotalBkg'))
            datahist = reader.read_hist_1d('{}/{}'.format(fit,'data_obs'))
            reader_stat = RootFileReader(os.path.join(directory_input,
                                'signalregion_{}_{}_stat.root'.format(cat,fit)))
            tzqhist_stat = reader_stat.read_hist_1d('{}/{}'.format('prefit','TotalSig'))
            bckhist_stat = reader_stat.read_hist_1d('{}/{}'.format('prefit','TotalBkg'))
            # (note: statistical uncertainties are taken from prefit!)
        
            # create variables
            bdt = Variable('BDT score', is_independent=True, is_binned=False)
            bdt.values = datahist['x']
            tzq = Variable('Number of tZq signal events', is_independent=False, is_binned=False)
            tzq.values = tzqhist['y']
            tzq_unc = Uncertainty('Total uncertainty on tZq signal' )
            tzq_unc.values = tzqhist['dy']
            tzq.add_uncertainty( tzq_unc )
            tzq_unc_stat = Uncertainty('Statistical uncertainty on tZq signal')
            tzq_unc_stat.values = tzqhist_stat['dy']
            tzq.add_uncertainty( tzq_unc_stat )
            bck = Variable('Number of background events', is_independent=False, is_binned=False)
            bck.values = bckhist['y']
            bck_unc = Uncertainty('Total uncertainty on background' )
            bck_unc.values = bckhist['dy']
            bck.add_uncertainty( bck_unc )
            bck_unc_stat = Uncertainty('Statistical uncertainty on background')
            bck_unc_stat.values = bckhist_stat['dy']
            bck.add_uncertainty( bck_unc_stat )
            data = Variable('Number of data events', is_independent=False, is_binned=False)
            data.values = datahist['y']
            data_unc = Uncertainty('Total uncertainty on data' )
            data_unc.values = datahist['dy']
            data.add_uncertainty( data_unc )

            # add variables to table
            table.add_variable(bdt)
            table.add_variable(tzq)
            table.add_variable(bck)
            table.add_variable(data)
        
            # add table to submission
            submission.add_table(table)

    ### get correlation matrix
    # update: apparantly not required in a hepdata submission?
    '''table = Table('Covariance matrix')
    descr = 'Covariance matrix of total fit'
    table.description = descr
    table.location = 'not in paper'
    #reader = RootFileReader(os.path.join(directory_input,
    #                            'fitDiagnostics_inclusive.root'))
    #covhist = reader.read_hist_2d('covariance_fit_s')
    covhist = read_hist_2d_labeled(os.path.join(directory_input,
                                'fitDiagnostics_inclusive.root'),
                                'covariance_fit_s')
    xax = Variable("x-axis", is_independent=True, is_binned=False)
    xax.values = covhist["x"]
    yax = Variable("y-axis", is_independent=True, is_binned=False)
    yax.values = covhist["y"]
    cov = Variable("Covariance", is_independent=False, is_binned=False)
    cov.values = covhist["z"]
    table.add_variable(xax)
    table.add_variable(yax)
    table.add_variable(cov)
    submission.add_table(table)'''

    ### create the output files
    print('writing output files...')
    if os.path.exists(directory_output):
        os.system('rm -r {}'.format(directory_output))
    os.makedirs(directory_output)
    submission.create_files(directory_output)
