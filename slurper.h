
#include <stdio.h>
#include <iostream>
#include <limits>
#include <sstream>
#include <fstream>
#include <iomanip>

#include "typedefs.h"
#include "utilities.h"

class Slurper {	//(inherit me)
public:		//use std::ofstream to write to file
	void print(const T_s& block="",std::ostream& output=std::cout) const { connections.print(block,output); }
protected:	//so that no one could instantiate this
	Slurper(const T_s& filename) { readout(filename); }
    virtual ~Slurper(void) { }	//destructor should be defined
	
	void readout(const T_s& filename);	//clears contents and fills them from new file
//	void AddBlock(const T_s& block_name, const T_mss& block_contents);	//full name is used
	void get(const T_s& name, T_mss& fields) const;
	void get_blocks(T_ls& names, const T_s& parent="") const;

	std::multimap<T_s,T_mss> blocks;	//{block_name:{block:contents}} multimap to allow same names on different levels

	static const T_mss delimiters; //defines what delimits fields
private:
	static const T_mss comments;
	static const T_mss strings;
	static const T_s whitespace;
	static const T_pss block_delimiters;

	static T_s& cut(T_s& result, T_mss::const_iterator& cm_status, T_mss::const_iterator& str_status);	//cuts whitespaces and comments
	static std::size_t skip_to_unopened(const T_s& str, const std::size_t pos, const T_pss& pr);	//curly brackets can be used inside
	static std::size_t find_first_in(const T_s& str, const std::size_t at_pos, const T_mss& from_this, T_mss::const_iterator& what_found);

	void parse(const T_s& from, const std::multimap<T_s,T_mss>::iterator* parent=NULL);	//writes into blocks and connections

	class Connections {
	public:
		Connections(void) : current(NULL) { }
		~Connections(void) { clear(); }
		
		void clear(void);
		void print(const T_s& str="",std::ostream& output=std::cout) const;
		bool unique(void);	//true if no nodes were deleted
		bool merge(std::multimap<T_s,T_mss>& blocks);	//true if some nodes were merged

		const std::multimap<T_s,T_mss>::iterator* AddChild(const std::multimap<T_s,T_mss>::iterator* parent=NULL, const std::multimap<T_s,T_mss>::iterator* child=NULL);	//also creates nodes
		static T_s GetSimpleName(const T_s& name, T_ls& residue);
		void FillFields(const T_s& full_name, T_mss& fields) const;	//block names are merged into field names
		void FillBlockNames(const T_s& full_parent_name, T_ls& children_names) const;
	private:
		class tree_node;	//forward declaration
		
		std::list<tree_node*> heads;
		mutable tree_node* current;
		
		tree_node* find(std::multimap<T_s,T_mss>::iterator itr) const;	//only current changes
		tree_node* find_name(const T_s& name) const;	//try templating later
		T_s GetName(std::multimap<T_s,T_mss>::iterator itr) const;	//returns full name
	} connections;
};

class Slurper::Connections::tree_node {
public:
	tree_node(std::multimap<T_s,T_mss>::iterator cns) : parent(NULL), contents(cns) { }
	~tree_node(void);
	
	bool equal(std::multimap<T_s,T_mss>::iterator itr) const { return itr==contents; }
	bool equal(const T_s& str) const { return str==contents->first; }
	tree_node* find(std::multimap<T_s,T_mss>::iterator itr) const;
	tree_node* find_name(const T_s& name) const;	//try templating later
	void find_names(const T_s& name, std::list<tree_node*>& result) const;	//searches only children
	tree_node* find_chain(const T_s& full_name);	//returns last in chain if found; almost const, but can return this
	void fill_fields(T_mss& fields, const T_s& to_append="") const;
	void fill_children_names(T_ls& result) const;
	void print(unsigned int tab=0,std::ostream& output=std::cout) const;	//prints all nodes down with their contents

	void add_child(tree_node* child);
	const std::multimap<T_s,T_mss>::iterator& get(void) const { return contents; }
	
	bool unique(void);	//recursively checks if contents are unique
	bool unique_value(std::multimap<T_s,T_mss>::iterator itr, bool& found);	//true if tree contains 1 or 0 such values (deletes duplicates)
	bool unique_values(tree_node* base);	//checks this->values against base->values and erases from base
	bool merge(std::multimap<T_s,T_mss>& blocks);	//blocks are needed to erase one of merged iterators
	void merge_with(tree_node* base);	//invalidates base
	
	void append_ancestors(T_s& result) const;
	static void cut_name(const T_s& name, T_ls& result);
private:
	tree_node* parent;
	std::multimap<T_s,T_mss>::iterator contents;
	std::list<tree_node*> children;
	
	static void print_tab(unsigned int tab,std::ostream& output=std::cout) { for(unsigned int i=0;i<tab;++i) output<<"\t"; };
	static const T_ls delimiters;
};
