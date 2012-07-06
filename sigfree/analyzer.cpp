#include <map>
#include <iostream>

#include "analyzer.h"
#include "macros.h"


template < class Type >
	Type max ( Type a, Type b )
	{
		return a > b ? a:b;
	}

/*SGAnalyzer::SGAnalyzer( EIFG* g, State* s )
{
	ifg = g;
	states = s;
}
*/

//detect DD anomaly in parents of vertex with offset "o"
int SGAnalyzer::parentAnomalyDD( int o,  int reg )
{
        std::map< int, std::vector< StateLink > >::iterator iter;
        std::map< int, ParCh >::iterator parent_ptr;
	int r = 0;

        iter = states->states.find( reg );
        if ( iter == states->states.end() ) return 0;

        parent_ptr = ifg->ParentChildArray.find( o );
        if ( parent_ptr == ifg->ParentChildArray.end() ) return 0;
        if ( (*parent_ptr).second.parent.size() == 0 ) return 0;
        if ( (*iter).second.size() == 0 ) return 0;

        for( unsigned int i = 0; i < (*parent_ptr).second.parent.size(); i++ )
        {
                std::vector< StateLink > & vec = (*iter).second;
                for ( unsigned int j = 0; j < vec.size(); j++ )
                {
                        if ( ((*parent_ptr).second.parent[i] == vec[j].inst_off) && 
                                ( vec[j].st == DEFINED || vec[j].st == DD ) )
                        {
                                if( vec[j].st == DD ) r += parentAnomalyDD( vec[j].inst_off, reg );
                                if ( j >= vec.size() ) { 
					if( vec.size() != 0 ) j = vec.size()-1;
					else j=0;
				}
                                if ( j < vec.size()  ) {
                                        ifg->addRelativeEdges( vec[j].inst_off );
                                        ifg->deleteFamilyRelations( vec[j].inst_off );
                                        ifg->removeVertex( vec[j].inst_off );
                                        vec.erase( vec.begin() + j );
                                        j--; // XXX[sadie] : previous statement requires this, othervise the loop skips one element
                                        r++;
                                }
                        }
                }
        }
        return r;
        
}

void SGAnalyzer::pruningUseless(  )
{
        std::map< int, std::vector< StateLink > >::iterator iter, iter2;
		int k;
		
		if( !states ) {
			PRINT_WARNING("There is no states! ");
			return;
		}

        for ( iter = states->states.begin(); iter != states->states.end(); iter++ )
        {
			for( int i = 0; i < (int) (*iter).second.size(); i++ )
            {
				if ( (*iter).second[i].st == DD )
                {
					(*iter).second[i].st = DEFINED;
                    i -= parentAnomalyDD( (*iter).second[i].inst_off, (*iter).first );
					if ( i < 0 ) i = 0;			
				}
				else if( (*iter).second[i].st == DU )
				{
					(*iter).second[i].st = UNDEFINED;
					i -= parentAnomalyDD( (*iter).second[i].inst_off, (*iter).first );
					if ( i < 0 ) i = 0;
				}
				else if ( (*iter).second[i].st == UR )
				{
					ifg->addRelativeEdges( (*iter).second[i].inst_off );
					ifg->deleteFamilyRelations( (*iter).second[i].inst_off );
					k = ifg->getVertexOp1( (*iter).second[i].inst_off );
					ifg->removeVertex( (*iter).second[i].inst_off );
					iter2 = states->states.find( k );
					if ( k!= (*iter).first && iter2 != states->states.end() )
					{
						for(unsigned int j = 0; j < (*iter2).second.size(); j++ )
						{
							if( (*iter2).second[j].inst_off == k )
							{
								(*iter2).second.erase( (*iter2).second.begin() + j );
							}
						}
					}
					(*iter).second.erase( (*iter).second.begin() + i );
					i--;
				}
			}
        }
	return;       
}

int SGAnalyzer::checkUsefull(int o )
{
	std::map< int, ParCh >::iterator iter, ch_iter;
	std::set< int, Comp >::iterator ptr;
	int usefull = 1;
	int len = 0;
	iter = ifg->ParentChildArray.find( o );
	if ( iter == ifg->ParentChildArray.end() ) return usefull;

	visited.insert( o );

	for (unsigned int i = 0; i < (*iter).second.child.size(); i++ )
	{
		ptr = visited.find( (*iter).second.child[i] );
		if ( ptr != visited.end() )
		{
			ifg->removeEdge( (*iter).first, (*ptr) );
		}
		else	
		{
			int tmp_len = checkUsefull( (*iter).second.child[i] );
			len = std::max( len, tmp_len );
		}
	}
	usefull += len;
	visited.erase( o );
	return usefull;
}


void SGAnalyzer::fill_border( int first, int o, int max_offset)
{
	std::set< int > tmp_border;
	int last;
	std::map< int, std::set< int > >::iterator b_iter;

        last = std::max( o, max_offset);

        b_iter = border.find( first );
        if ( b_iter == border.end() )
        {
	        tmp_border.clear();
                tmp_border.insert( last );
                border.insert( std::pair< int, std::set<int> > ( first, tmp_border ) );
        }
        else
        {
        	(*b_iter).second.insert( last );
        }

}

int SGAnalyzer::checkUsefull( int length, int o, int first, int max_offset )
{
	std::map< int, ParCh>::iterator iter;
	std::set< int, Comp >::iterator ptr;
	std::map< int, std::set< int > >::iterator b_iter;
	int usefull  = 1;
	int len = 0;
	int last;
	std::set< int > tmp_border;
	
	iter = ifg->ParentChildArray.find(o);
	if ( iter == ifg->ParentChildArray.end() ) return 0; 

	// if we met the last instruction in the chain which exeeds threshold
	if ( (*iter).second.child.empty() && length <= 1)
	{
		fill_border( first, o, max_offset );
	}
	
	// that helps to escape infinite cycles
	visited.insert( o );

        for (unsigned int i = 0; i < (*iter).second.child.size(); i++ )
        {
                ptr = visited.find( (*iter).second.child[i] );
                if ( ptr != visited.end() )
                {
			if ( length <=1 ) fill_border( first, o, max_offset);
                        ifg->removeEdge( (*iter).first, (*ptr) );
                }
                else
                {
			max_offset = std::max( max_offset, o);
                        int tmp_len = checkUsefull( length-1, (*iter).second.child[i], first, max_offset );
                        len = std::max( len, tmp_len );
                }
        }
        usefull += len;
        visited.erase( o );
        return usefull;

}

int SGAnalyzer::isExecutable (  )
{
	std::map< int, ParCh >::iterator iter;
	int us_len = 0;
	int count = 0;

	for ( iter = ifg->ParentChildArray.begin(); iter!= ifg->ParentChildArray.end(); ++iter)
	{
		// if offset responds to first instruction in some chain
		if( (*iter).second.parent.empty() )
		{
			int u = checkUsefull ( SCHEME2_THR, (*iter).first, (*iter).first, -1 );
			us_len = std::max( us_len, u );
		}
	}
	if ( us_len > threshold ) return us_len;
	return 0;
}


std::map< int, std::set<int> >* SGAnalyzer::getBorder()
{
	return &border;
}
