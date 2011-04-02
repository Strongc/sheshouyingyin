/*******************************************************************************
Tree Container Library: Generic container library to store data in tree-like structures.
Copyright (c) 2006  Mitchel Haas

This software is provided 'as-is', without any express or implied warranty. 
In no event will the author be held liable for any damages arising from 
the use of this software.

Permission is granted to anyone to use this software for any purpose, 
including commercial applications, and to alter it and redistribute it freely, 
subject to the following restrictions:

1.	The origin of this software must not be misrepresented; 
you must not claim that you wrote the original software. 
If you use this software in a product, an acknowledgment in the product 
documentation would be appreciated but is not required.

2.	Altered source versions must be plainly marked as such, 
and must not be misrepresented as being the original software.

3.	The above copyright notice and this permission notice may not be removed 
or altered from any source distribution.

For complete documentation on this library, see http://www.datasoftsolutions.net
Email questions, comments or suggestions to mhaas@datasoftsolutions.net
*******************************************************************************/

template<typename stored_type, typename tree_type, typename container_type, typename tree_category_type>
tcl::const_pre_order_descendant_node_iterator<stored_type, tree_type, container_type, tree_category_type>& tcl::const_pre_order_descendant_node_iterator<stored_type, tree_type, container_type, tree_category_type>::operator ++()
{
	if ( !it->empty() ) { // any children?
		node_stack.push(it); // yes. push current pos
		it = it->node_begin(); // and goto first child
	} else {
		++it; // no children. incr to next sibling if present
		// while stack not empty and no next sibling
		while ( !node_stack.empty() && it == (node_stack.top())->node_end() ) {
			it = node_stack.top(); // pop parent
			node_stack.pop();
			++it; // and see if it's got a next sibling
		}
	}
	return *this; 
}


template<typename stored_type, typename tree_type, typename container_type, typename tree_category_type>
tcl::const_pre_order_descendant_node_iterator<stored_type, tree_type, container_type, tree_category_type>& tcl::const_pre_order_descendant_node_iterator<stored_type, tree_type, container_type, tree_category_type>::operator --()
{
	typedef typename tree_category_type::const_node_iterator iterator_type;
	if ( it == pTop_node->node_end() ) { // at end?
		// yes. need to set up stack to state just before end
		rit = pTop_node->children.rbegin(); // going backwards
		if ( rit != pTop_node->children.rend() ) { // insure there's children
			if ( !(*rit)->empty() ) { // last node have children?
				do {  // find the last child of this node
					++rit; // incr reverse iter..
					it = iterator_type(rit.base(), (it != pTop_node->node_end() ? &(*it) : pTop_node)); // ..to convert to fwd iter correctly
					node_stack.push(it); // push parents on the way down
					rit = it->children.rbegin(); // get last child again
				} while ( !(*rit)->empty() ); // while last child has children
			}
			++rit; // incr reverse iter
			it = iterator_type(rit.base(), (it != pTop_node->node_end() ? &(*it) : pTop_node)); // to convert to forward iter correctly
		}
	} else { // not at end.
		if ( it != it->parent()->node_begin() ) { // is this first sibling?
			--it; // no.  ok to decr to next sibling
			if (!it->empty()) { // children present?
				do { // yes.  get deepest last child
					node_stack.push(it); // first push current 
					it = iterator_type(it->children.end(), &(*it));
					--it;  // then go to last child
				} while ( !it->empty() ); // while children present
			}
		} else { // first sibling
			it = node_stack.top(); // just need to goto parent
			node_stack.pop();
		}
	}
	return *this;
}

template<typename stored_type, typename tree_type, typename container_type, typename tree_category_type>
tcl::const_post_order_descendant_node_iterator<stored_type, tree_type, container_type, tree_category_type>::const_post_order_descendant_node_iterator(const tree_category_type* pTop_node_)
{
	pTop_node = pTop_node_; // save invoked node
	it = pTop_node->node_begin(); // goto first child
	if ( it != pTop_node->node_end()) {
		if ( !it->empty() ) { // have children of it's own?
			do {  // goto deepest first child, while pushing parents
				node_stack.push(it);
				it = it->node_begin();
			} while ( !it->empty() );
		}
	}
}

template<typename stored_type, typename tree_type, typename container_type, typename tree_category_type>
tcl::const_post_order_descendant_node_iterator<stored_type, tree_type, container_type, tree_category_type>& tcl::const_post_order_descendant_node_iterator<stored_type, tree_type, container_type, tree_category_type>::operator ++()
{
	const typename tree_category_type::const_node_iterator it_end = it->parent()->node_end(); // end sibling
	++it; // advance to next sibling, if present
	if ( it != it_end && !it->empty() ) { // next sibling present, and has children?
		do {  // goto deepest first child while pushing parents
			node_stack.push(it);
			it = it->node_begin();
		} while ( !it->empty() );
	} else { // it is past last sibling, or it has no children
		// if valid it and it has no childrent, were done
		if ( !node_stack.empty() && it == node_stack.top()->node_end() ) {
			// it is past last sibling, and pushed parents exist.  move back up to parent
			it = node_stack.top();
			node_stack.pop();
		}
	}
	return *this;
}

template<typename stored_type, typename tree_type, typename container_type, typename tree_category_type>
tcl::const_post_order_descendant_node_iterator<stored_type, tree_type, container_type, tree_category_type>& tcl::const_post_order_descendant_node_iterator<stored_type, tree_type, container_type, tree_category_type>::operator --()
{
	typedef typename tree_category_type::const_node_iterator iterator_type;
	if ( it == pTop_node->node_end() ) { // at end?
		typename container_type::const_reverse_iterator rit = pTop_node->children.rbegin();
		++rit;
		it = iterator_type(rit.base(), pTop_node); // goto last sibling of top node
	} else { // not at end
		if ( !it->empty() ) { // children present?
			typename container_type::const_reverse_iterator rit = it->children.rbegin();
			node_stack.push(it);
			++rit; // push parent and go to last child
			it = iterator_type(rit.base(), &(*it));
		} else { // no children present
			if ( it != it->parent()->node_begin() ) { // at first sibling?
				--it; // no.  just goto prev sibling
			} else { // at first sibling. work our way up until not first sibling
				while ( !node_stack.empty() && it == node_stack.top()->node_begin())
				{
					it = node_stack.top();
					node_stack.pop();
				}
				--it; // then goto prev sibling
			}
		}
	}
	return *this;
}

template<typename stored_type, typename tree_type, typename container_type, typename tree_category_type>
tcl::const_level_order_descendant_node_iterator<stored_type, tree_type, container_type, tree_category_type>& tcl::const_level_order_descendant_node_iterator<stored_type, tree_type, container_type, tree_category_type>::operator ++()
{
	const typename tree_category_type::const_node_iterator it_end = it->parent()->node_end(); 
	node_queue.push(it); // push current pos node in queue
	++it;  // and goto next sibling if present

	if ( it == it_end ) { // past last sibling?  If not, we're done.
		while ( !node_queue.empty() ) { // yes. Insure queue not empty
			it = node_queue.front(); // pull pos off queue
			node_queue.pop(); // this should be the start pos of level just traversed
			if ( !it->empty() ) { // have children?
				it = it->node_begin(); // yes. descend to start of next level
				break;
			} else if ( node_queue.empty() ) { // no children.  is queue empty?
				it = pTop_node->node_end(); // yes. at end
				return *this;
			}
		} 
	}
	return *this;
}
