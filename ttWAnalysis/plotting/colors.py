######################
# tools for plotting #
######################

import ROOT


def getcolormap( style='default' ):
  ### get a color map (see definitions below)
  style=style.lower()
  if(style=='default'): return getcolormap_default()
  else: print('WARNING in getcolormap: style not recognized, returning None')
  return None

def define_color_hex( hexstring ):
  ### define a color based on a hexadecimal encoding.
  # note: see e.g. here for choosing colors:
  #       https://htmlcolorcodes.com/  
  # note: the color object needs to stay in scope, else the index will refer to a nullptr
  #       and the color will turn out white...
  #       this is achieved by setting the color as a global ROOT property.
  r, g, b = tuple(int(hexstring[i:i+2], 16) for i in (1, 3, 5))
  cindex = ROOT.TColor.GetFreeColorIndex()
  color = ROOT.TColor(cindex, r/255., g/255., b/255.)
  setattr(ROOT,'temp_color_'+str(cindex),color)
  return cindex,color

def getcolormap_default():
    # map of histogram titles to colors
    cmap = {}
    cmap['nonprompt'] = define_color_hex('#ffe380')[0]
    cmap['DY'] = define_color_hex('#ffd22e')[0]
    cmap['TT'] = define_color_hex('#ffbd80')[0]
    cmap['TTX'] = define_color_hex('#4e09be')[0]
    cmap['TTZ'] = define_color_hex('#336fce')[0]
    cmap['TTW'] = define_color_hex('#2f8ceb')[0]
    cmap['TTTT'] = define_color_hex('#ff0000')[0]
    #cmap['tZq'] = define_color_hex('#ff0000')[0]
    cmap['WZ'] = define_color_hex('#81efd7')[0]
    cmap['ZZ'] = define_color_hex('#2fbc6c')[0]
    cmap['ZZH'] = cmap['ZZ']
    cmap['ZG'] = define_color_hex('#9c88ff')[0]
    cmap['XG'] = cmap['ZG']
    cmap['triboson'] = define_color_hex('#c6ff00')[0]
    cmap['Triboson'] = cmap['triboson']
    cmap['Other'] = define_color_hex('#ccccaa')[0]
    return cmap
