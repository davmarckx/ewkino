###############################################################################
# Tools for propagating uncertainties from absolute to normalized measurement #
###############################################################################

import sys
import os
import copy
import numpy as np

def dict_to_array(d):
  ### convert a 2D dict to an array
  keys = sorted(d.keys())
  nkeys = len(keys)
  a = np.zeros((nkeys,nkeys))
  for i,key1 in enumerate(keys):
    for j,key2 in enumerate(keys):
      a[i,j] = d[key1][key2]
  return a

def dfivec(i, xi, xsum, xlen):
    ### calculate the vector of dfi/dx
    dfi = np.ones(xlen)*(-xi/xsum**2)
    dfi[i] += 1./xsum
    return dfi

def sigmafi(dfi, cov):
    ### calculate the sigma_f for a given covariance matrix
    sigmafi2 = np.inner(np.inner(dfi,cov),dfi)
    if sigmafi2<0:
      msg = 'WARNING in uncertaintyprop.py / sigmafi:'
      msg += ' variance is negative, cannot calculated std, returning 0.'
      print(msg)
      return 0.
    return sigmafi2**(0.5)

def normalize(xsecs, covs):
    ### calculate full set of normalized values and uncertainties
    # input arguments:
    # - xsecs: 1D numpy array of (non-normalized) differential cross-section values
    # - covs: list of covariance matrices.
    #   each matrix must be a 2D numpy array, with shape and order corresponding to xsecs.
    # output:
    # 2D numpy array, where each row is of the form [central, error_cov1, ..., error_covn]
    xlen = len(xsecs)
    xsum = float(np.sum(xsecs))
    res = np.zeros((xlen,len(covs)+1))
    res[:,0] = xsecs/xsum
    for i in range(xlen):
        dfi = dfivec(i,xsecs[i], xsum, xlen)
        for j,cov in enumerate(covs):
            res[i,j+1] = sigmafi(dfi, cov)
    return res

def normalizexsec(xsec, ignorecov=False):
    ### calculate full set of normalized values and uncertainties
    # input arguments:
    # - xsec: dict outputted by sstoxsec,
    #   i.e. one with keys 'pois' (holding a dict of poi names to xsec values+errors)
    #   and other keys holding double dicts of covariance matrices.
    # output
    # dict in the same format as xsec['pois']
    pois = sorted(xsec['pois'].keys())
    xsecs = np.array([xsec['pois'][poi][0] for poi in pois])
    covs = []
    keys = ['statcovdown', 'statcovup', 'syscovdown', 'syscovup']
    for key in keys:
        if key in xsec.keys():
          covarray = dict_to_array(xsec[key])
          if ignorecov:
            # set all nondiagonal elements to zero
            for i in range(covarray.shape[0]):
              for j in range(i+1,covarray.shape[1]):
                covarray[i,j] = 0
                covarray[j,i] = 0
          covs.append(covarray)
    normxsecs = normalize(xsecs, covs)
    res = {}
    for i,poi in enumerate(pois):
        res[poi] = list(normxsecs[i,:])
    return res

def sstoxsec(ss, pred):
    ### convert signal strengths to xsections, with uncertainties and covariances
    # input parameters:
    # - ss: dict outputted by combine/differential_readoutput per variable,
    #   i.e. one with keys 'pois' (holding a dict of poi names to values+errors)
    #   and other keys holding double dicts of covariance matrices.
    # - pred: dict mapping poi names to predicted values for the cross-section.
    # output:
    # dict with same structure as ss but everything in absolute values
    # rather than signal strengths
    pois = ss['pois'].keys()
    if sorted(pois)!=sorted(pred.keys()):
        print(pois)
        print(pred.keys())
        raise Exception('ERROR in uncertaintyprop.py / sstoxsec:'
          + ' keys of provided signal strengths and predictions do not agree.')
    res = {}
    res['pois'] = {}
    # multiply individual values by the scalar prediction
    for poi in pois:
         res['pois'][poi] = [el*pred[poi] for el in ss['pois'][poi]]
    # multiply covariance matrices by combinations of predictions
    for key in ss.keys():
        if key=='pois': continue
        cov = ss[key]
        normcov = copy.deepcopy(cov)
        for poi1 in pois:
            for poi2 in pois:
                normcov[poi1][poi2] = cov[poi1][poi2]*pred[poi1]*pred[poi2]
        res[key] = normcov
    return res


if __name__=='__main__':
    # testing section

    # only one variable with one uncertainty
    xsecs = np.array([1])
    cov = np.array([[0.5]])
    xsecsnorm = normalize(xsecs,[cov])
    print('--- Test 1: ---')
    print('xsecs:')
    print(xsecs)
    print('cov:')
    print(cov)
    print('xsecsnorm:')
    print(xsecsnorm)

    xsecs = np.array([1,1])
    cov = np.array([[0.01,0],[0,0.01]])
    xsecsnorm = normalize(xsecs,[cov])
    print('--- Test: ---')
    print('xsecs:')
    print(xsecs)
    print('cov:')
    print(cov)
    print('xsecsnorm:')
    print(xsecsnorm)
