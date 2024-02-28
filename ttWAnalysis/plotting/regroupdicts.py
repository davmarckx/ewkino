##########################
# Extra process grouping #
##########################
# Note: This comes on top of the process grouping 
#       in merge.py (with a rename_processes.json file).
#       It has no impact on the input ROOT file,
#       only on the resulting plots.
#       This allows to have cleaner plots while still
#       having a finer splitting for combine.
#       It also allows to have different groupings
#       per event selection region.
# Note: As an input, use the names that come out of the merging step
#       (i.e. the values of rename_processes.json).
#       But also take into account the renaming that is done
#       according to infodicts.py, that is run before this step!

def get_regroup_process_dict(groupid='default'):
    pdict = {}
    if groupid == 'nogroup': pass
    else:
        # default case
        pdict['Nonprompt (e)'] = 'Nonprompt'
        pdict['Nonprompt (mu)'] = 'Nonprompt'
        pdict['ZZ'] = 'Multiboson'
        pdict['WZ'] = 'Multiboson'
        pdict['TTG'] = 'Conversions'
        pdict['ZG'] = 'Conversions'
        pdict['TX'] = 'T(T)X'
        pdict['TTX'] = 'T(T)X'
    return pdict
