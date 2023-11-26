#include "samplerate_handler.hxx"

sample_rate_list::sample_rate_list():
  don_t_care( true)
{}
void sample_rate_list::add_value(const unsigned short&a)
{
  don_t_care = false;
  if( find( list_sr.begin(), list_sr.end(), a ) == list_sr.end())
	list_sr.insert( a );
}
sample_rate_list sample_rate_list::intersect_sr_list(const sample_rate_list&a)const
{
  sample_rate_list the_return;
  if ( don_t_care )
	{
	  // copy form a
	  the_return.don_t_care = a.don_t_care;
	  the_return.list_sr = a.list_sr;
	}else{
	the_return.don_t_care = false;
	if ( a.don_t_care )
	  the_return.list_sr = list_sr;
	else
	  // search for the intersection
	  // no set_intersection as it came with C++17
	  for_each( list_sr.begin(), list_sr.end(), [&](const unsigned short& b){
		  auto iter = find( a.list_sr.begin(), a.list_sr.end(), b );
		  if ( iter != a.list_sr.end() )
			the_return.list_sr.insert( b );
		});
  }
	return the_return;
}
pair<unsigned short,bool> sample_rate_list::get_sample_rate()const
{
  // default strategy is to return the lowest one
  if( don_t_care )
	// return 4;
	return make_pair(48,false);
  else
	if ( list_sr.begin() == list_sr.end() )
	  // list is empty, return 0
	  return make_pair(0,true);
	else
	  {
		const unsigned short xxx=*list_sr.begin();
		return make_pair(xxx,xxx<48);
	  }
}
ostream&operator<<(ostream&a,const sample_rate_list&b)
{
  if ( b.don_t_care )
	a<<"[all]";
  else
	if ( b.list_sr.begin() == b.list_sr.end() )
	  a<<"[none]";
	else
	  for_each( b.list_sr.begin(), b.list_sr.end(), [&a](const unsigned short&c)
				{
				  a<<" "<<c*1000<<" ";
				});
  return a;
}

