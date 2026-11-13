import datetime as dt, numpy as np
def nth_sunday(y, m, n):
    d = dt.date(y, m, 1); d += dt.timedelta(days=(6 - d.weekday()) % 7); return d + dt.timedelta(days=7*(n-1))
def last_sunday(y, m):
    d = dt.date(y, m, 31) if m in (3,10,12,1,5,7,8) else dt.date(y, m, 30)
    return d - dt.timedelta(days=(d.weekday()+1) % 7)
def us_dates(y): return nth_sunday(y,3,2), nth_sunday(y,11,1)
def eu_dates(y): return last_sunday(y,3), last_sunday(y,10)
def day_ord(d): return (d - dt.date(1970,1,1)).days
def cell_of_dayord(dord):
    """dord: array of days since epoch (label date). returns (us_dst, eu_dst) bool arrays."""
    d = np.asarray(dord)
    years = (d.astype('datetime64[D]').astype('datetime64[Y]').astype(int) + 1970)
    us = np.zeros(len(d), bool); eu = np.zeros(len(d), bool)
    for y in np.unique(years):
        m = years == y
        a, b = us_dates(y); us[m] = (d[m] >= day_ord(a)) & (d[m] < day_ord(b))
        a, b = eu_dates(y); eu[m] = (d[m] >= day_ord(a)) & (d[m] < day_ord(b))
    return us, eu
def uk_dst_utc(y):
    a, b = eu_dates(y)
    f = lambda d: int(dt.datetime(d.year, d.month, d.day, 1, tzinfo=dt.timezone.utc).timestamp())
    return f(a), f(b)
