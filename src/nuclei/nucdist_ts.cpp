/*
  -------------------------------------------------------------------
  
  Copyright (C) 2006-2014, Andrew W. Steiner
  
  This file is part of O2scl.
  
  O2scl is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 3 of the License, or
  (at your option) any later version.
  
  O2scl is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.
  
  You should have received a copy of the GNU General Public License
  along with O2scl. If not, see <http://www.gnu.org/licenses/>.

  -------------------------------------------------------------------
*/
#include <o2scl/constants.h>
#include <o2scl/nucdist.h>
#include <o2scl/hdf_nucmass_io.h>
#include <o2scl/test_mgr.h>

using namespace std;
using namespace o2scl;
using namespace o2scl_const;
using namespace o2scl_hdf;

int main(void) {
  test_mgr t;
  t.set_output_level(2);
   
  cout.setf(ios::scientific);

  // Load several mass formulae to make distributions out of
  
  nucmass_ame ame12;
  o2scl_hdf::ame_load(ame12,"");

  nucmass_ame_exp amex12;
  o2scl_hdf::ame_load(amex12,"");

  nucmass_mnmsk mth;
  o2scl_hdf::mnmsk_load(mth,"");

  nucmass_mnmsk_exp mexp;
  o2scl_hdf::mnmsk_load(mexp,"");

  nucmass_ame ame03;
  o2scl_hdf::ame_load(ame03,"03");
  
  nucmass_ame_exp amex03;
  o2scl_hdf::ame_load(amex03,"03");

  // Test the size of the distributions

  nucdist_full fd(mth);
  size_t cnt=0;
  for(nucdist::iterator ndi=fd.begin();ndi!=fd.end();ndi++) {
    cnt++;
  }
  cout << "fd size: " << fd.size() << endl;
  t.test_gen(fd.size()==8979,"nucdist_full size 1");
  t.test_gen(cnt==8979,"nucdist_full size 1b");

  nucdist_full fd2(ame03);
  cout << "fd2 size: " << fd2.size() << endl;
  t.test_gen(fd2.size()==3178,"nucdist_full size 2");

  nucdist_full fd3(amex03);
  cout << "fd3 size: " << fd3.size() << endl;
  t.test_gen(fd3.size()==2227,"nucdist_full size 3");

  nucdist_full fd4(mexp);
  cout << "fd4 size: " << fd4.size() << endl;
  t.test_gen(fd4.size()==1654,"nucdist_full size 4");

  nucdist_full fd5(ame12);
  cout << "fd5 size: " << fd5.size() << endl;
  t.test_gen(fd5.size()==3352,"nucdist_full size 5");

  nucdist_full fd6(amex12);
  cout << "fd6 size: " << fd6.size() << endl;
  t.test_gen(fd6.size()==2437,"nucdist_full size 6");

  t.report();
  return 0;
}
