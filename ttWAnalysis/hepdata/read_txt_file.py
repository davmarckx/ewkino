########################################
# help functions for reading txt files #
########################################
# COPIED FROM TZQ
# usage: txt files summarizing numerical results are already used for other purposes
#        such as plotting.
#        now try to read existing files to convert to proper format for HepData submission

def readchanneltxt(path_to_txtfile):
    ### read a txt file and return a list of channels that can be used for plotchannels
    # expected formatting of txt file: name label central statup statdown totup totdown
    # note: now more flexible allowing for any number of entries per channel,
    #       as long as this number is the same for each channel.
    # note: label may not contain spaces, but underscores can be used instead
    # note: lines in the file starting with # will be ignored
    channels = []
    nentries = -1
    f = open(path_to_txtfile,'r')
    for line in f:
        if line[0]=='#': continue
        line = line.strip(' ')
        channel = line.split(' ')
        if nentries>0 and len(channel)!=nentries:
            raise Exception('ERROR: inconsistent number of entries per channel'
                            +' while reading input file {}'.format(path_to_txtfle))
        nentries = len(channel)
        # replace underscores by spaces except when a subscript is implied
        channel[1] = channel[1].replace('_{','blablatemp')
        channel[1] = channel[1].replace('_',' ')
        channel[1] = channel[1].replace('blablatemp','_{')
        for i in range(len(channel))[2:]: channel[i] = float(channel[i])
        channels.append(channel)
    return channels
