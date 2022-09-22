######################################################################
# tools for dealing with lists of histogram variables in json format #
######################################################################

import sys
import os
import json

class HistogramVariable(object):

  def __init__( self, name, title, nbins, xlow, xhigh, 
                axtitle=None, unit=None, comments=None ):
    self.name = name
    self.title = title
    self.nbins = int(nbins)
    self.xlow = float(xlow)
    self.xhigh = float(xhigh)
    self.axtitle = axtitle
    self.unit = unit
    self.comments = comments
    self.ordered_keys = (['name','title','nbins','xlow','xhigh',
                          'axtitle','unit','comments'])

  def __str__( self ):
    res = 'HistogramVariable( '
    res += ', '.join(['{}: {}'.format(key,getattr(self,key)) for key in self.ordered_keys])
    res += ' )'
    return res


def check_json( jsonobj, verbose=True ):
  ### check the correct content of an object loaded from a json file
  if not isinstance( jsonobj, list ):
    if verbose: print('ERROR: json object should be a list, found {}'.format(type(jsonobj)))
    return False
  for el in jsonobj:
    if not isinstance( el, dict ):
      if verbose: print('ERROR: each element in the json object should be a dict,'
                        +' found {}'.format(type(el)))
      return False
    reqkeys = ['name','title','nbins','xlow','xhigh']
    optkeys = ['axtitle','unit','comments']
    for reqkey in reqkeys:
      if( reqkey not in el.keys() ):
        msg = 'ERROR: variable does not contain required key {};'.format(reqkey)
        msg += ' found {}'.format(el)
        if verbose: print(msg)
        return False
    for key in el.keys():
      if key not in reqkeys+optkeys:
        msg = 'ERROR: variable contains the key {}'.format(key)
        msg += ' which is not recognized.'
        if verbose: print(msg)
        return False
  return True

def read_variables( jsonfile ):
  ### read a collection of histogram variables
  # return type: list of HistogramVariables
  with open(jsonfile, 'r') as f:
    variables = json.load(f)
  content_ok = check_json( variables )
  if not content_ok:
    raise Exception('ERROR in parsing json files with variables,'
                    +' see more detailed error message above.')
  res = []
  for var in variables:
    res.append( HistogramVariable( var['name'], var['title'], var['nbins'],
                var['xlow'], var['xhigh'],
                axtitle=var.get('axtitle',None),
                unit=var.get('unit',None),
                comments=var.get('comments',None) ) )
  return res

def write_variables_txt( variables, txtfile ):
  ### write a collection of variables in plain txt format
  # (more useful than json for reading in c++)
  lines = []
  for var in variables:
    line_elements = []
    line_elements.append(var.name)
    line_elements.append(var.title)
    line_elements.append(str(var.nbins))
    line_elements.append(str(var.xlow))
    line_elements.append(str(var.xhigh))
    lines.append( ' '.join(line_elements) )
  with open(txtfile, 'w') as f:
    for line in lines:
      f.write(line+'\n')

def write_variables_json( variables, jsonfile, builtin=False ):
  ### write a collection of variables to json format
  varlist = []
  for var in variables:
    vardict = ({ 'name' : var.name,
                 'title' : var.title,
                 'nbins': var.nbins,
                 'xlow': var.xlow,
                 'xhigh': var.xhigh })
    if var.axtitle is not None: vardict['axtitle'] = var.axtitle
    if var.unit is not None: vardict['unit'] = var.unit
    if var.comments is not None: vardict['comments'] = var.comments
    varlist.append( vardict )
  if builtin:
    # use builtin json.dump
    # easier, but not easily readable
    with open(jsonfile, 'w') as f:
      json.dump(varlist, f)
  else:
    # manual parsing
    ordered_keys = (['name','title','nbins','xlow','xhigh',
                     'axtitle','unit','comments'])
    lines = []
    lines.append('[')
    for vardict in varlist:
      lines.append('{')
      for key in ordered_keys:
        if key in vardict: lines.append('  "{}": "{}",'.format(key,vardict[key]))
      lines[-1] = lines[-1].rstrip(',')
      lines.append('},')
    lines[-1] = lines[-1].rstrip(',')
    lines.append(']')
    with open(jsonfile, 'w') as f:
      for line in lines: f.write(line+'\n')
