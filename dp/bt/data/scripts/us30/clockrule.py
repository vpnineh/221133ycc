"""Verified HistData label clock -> UTC (see CLOCK_REPORT.md).
label = US Eastern local time with US DST for label dates < 2018-11-04 (UTC-4 from 2nd Sun Mar to 1st Sun Nov, else UTC-5);
label = UTC-5 plus London/EU summer time (UTC-4 from last Sun Mar to last Sun Oct, else UTC-5) from 2018-11-04 on.
All switches occur on a Sunday while the market is closed; the boundary is taken at 00:00 label on that Sunday."""
import numpy as np, datetime as dt
from rules import us_dates, eu_dates, day_ord
CUT = day_ord(dt.date(2018, 11, 4))
def label_offset_hours(t_label):
    """t_label: int64 label epoch seconds (label clock read as naive UTC). returns array of -4/-5 (label minus UTC, hours)."""
    t_label = np.asarray(t_label)
    d = t_label // 86400
    years = (d.astype('datetime64[D]').astype('datetime64[Y]').astype(int) + 1970)
    summer = np.zeros(len(d), bool)
    for y in np.unique(years):
        m = years == y
        ua, ub = us_dates(y); ea, eb = eu_dates(y)
        us_s = (d >= day_ord(ua)) & (d < day_ord(ub)); eu_s = (d >= day_ord(ea)) & (d < day_ord(eb))
        summer[m] = np.where(d[m] < CUT, us_s[m], eu_s[m])
    return np.where(summer, -4, -5)
def label_to_utc(t_label):
    return np.asarray(t_label) - label_offset_hours(t_label) * 3600
def uk_summer_mask_utc(t_utc):
    t_utc = np.asarray(t_utc)
    years = (t_utc.astype('datetime64[s]').astype('datetime64[Y]').astype(int) + 1970)
    m = np.zeros(len(t_utc), bool)
    for y in np.unique(years):
        a, b = eu_dates(y)
        f = lambda d: int(dt.datetime(d.year, d.month, d.day, 1, tzinfo=dt.timezone.utc).timestamp())
        s = years == y
        m[s] = (t_utc[s] >= f(a)) & (t_utc[s] < f(b))
    return m
