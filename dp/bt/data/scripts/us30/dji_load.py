import sys, pandas as pd, numpy as np
def load(path):
    df = pd.read_csv(path, sep='\t', dtype={'<DATE>': str, '<TIME>': str})
    ts = pd.to_datetime(df['<DATE>'] + ' ' + df['<TIME>'], format='%Y.%m.%d %H:%M:%S', errors='coerce')
    a = np.empty(len(df), dtype=[('t','<i8'),('o','<f8'),('h','<f8'),('l','<f8'),('c','<f8')])
    a['t'] = ts.values.astype('datetime64[s]').astype(np.int64)
    for k, col in zip('ohlc', ['<OPEN>','<HIGH>','<LOW>','<CLOSE>']): a[k] = df[col].values
    return a
