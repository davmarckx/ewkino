##########################
# Extra process grouping #
##########################
# Note: this comes on top of the process grouping 
#       in merge.py (with a rename_processes.json file).
#       it has no impact on the input ROOT file,
#       only on the resulting plots.
#       this allows to have cleaner plots while still
#       having a finer splitting for combine.
#       it also allows to have different groupings
#       per event selection region.

def get_regroup_process_dict(region=''):
    pdict = {}
    if region=='test':
        pass
    else:
        # default case
        pdict['ZZ'] = 'Multiboson'
        pdict['WZ'] = 'Multiboson'
        pdict['TTG'] = 'Conversions'
        pdict['ZG'] = 'Conversions'
    return pdict
