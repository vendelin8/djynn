

#include <stdlib.h>
#include <stdio.h>
#include "tree.h"


int q_tree_remove(QTreeNode *tree,QTreeNode node) {
	int ret = 1;
	if(node==NULL) return 0;
	if(tree!=NULL && *tree!=NULL) {
		if(*tree==node) *tree = node->sibling;
		else if(node->parent!=NULL && node->parent->child==node) node->parent->child = node->sibling;
		else {
			QTreeNode p = node->parent!=NULL? node->parent->child : *tree;
			for(; p->sibling!=node && p->sibling!=NULL; p=p->sibling);
			if(p->sibling==node) p->sibling = node->sibling;
			else ret = 0;
		}
	} else ret = 0;
	node->parent = node->sibling = NULL;
//fprintf(stderr,"q_tree_remove(%d)\n",ret);
	return ret;
}

int q_tree_insert_before(QTreeNode *tree,QTreeNode dest,QTreeNode node) {
	int ret = 1;
//fprintf(stderr,"q_tree_insert_before()\n");
	if(node==NULL || node==dest) return 0;
	q_tree_remove(tree,node);
	if(tree==NULL || *tree==NULL) return 0;
	if(dest!=NULL) {
		if(*tree==dest) node->sibling = dest,*tree = node;
		else if(dest->parent!=NULL && dest->parent->child==dest) dest->parent->child = node,node->sibling = dest;
		else {
			QTreeNode s = dest->parent!=NULL? dest->parent->child : *tree;
			for(; s->sibling!=dest && s->sibling!=NULL; s=s->sibling);
			if(s->sibling==dest) s->sibling = node,node->sibling = dest;
			else ret = 0;
		}
		node->parent = dest->parent;
	} else node->sibling = *tree,*tree = node;
//fprintf(stderr,"q_tree_insert_before(%d)\n",ret);
	return ret;
}

int q_tree_insert_after(QTreeNode *tree,QTreeNode dest,QTreeNode node) {
	int ret = 1;
//fprintf(stderr,"q_tree_insert_after()\n");
	if(node==NULL || node==dest) return 0;
	q_tree_remove(tree,node);
	if(dest!=NULL) node->parent = dest->parent,node->sibling = dest->sibling,dest->sibling = node;
	else if(tree!=NULL) {
		if(*tree!=NULL) {
			dest = *tree;
			while(dest->sibling!=NULL) dest = dest->sibling;
			node->sibling = NULL,dest->sibling = node;
		} else *tree = node;
	} else ret = 0;
//fprintf(stderr,"q_tree_insert_after(%d)\n",ret);
	return ret;
}

int q_tree_insert_child(QTreeNode *tree,QTreeNode dest,QTreeNode node) {
	int ret = 1;
//fprintf(stderr,"q_tree_insert_child()\n");
	if(node==NULL || node==dest) return 0;
	q_tree_remove(tree,node);
	if(dest!=NULL) node->parent = dest,node->sibling = dest->child,dest->child = node;
	else if(tree!=NULL && *tree!=NULL) *tree = node;
	else ret = 0;
//fprintf(stderr,"q_tree_insert_child(%d)\n",ret);
	return ret;
}

int q_tree_count(QTreeNode tree) {
	int n = 0;
	for(; tree!=NULL; ++n,tree=tree->sibling)
		if(tree->child!=NULL)
			n += q_tree_count(tree->child);
	return n;
}

int q_tree_contains(QTreeNode tree,QTreeNode node) {
	if(tree!=NULL && node!=NULL) {
		QTreeNode n = tree;
		while(n!=NULL) {
			if(n==node) return 1;
			if(n->child!=NULL) n = n->child;
			else if(n->sibling!=NULL) n = n->sibling;
			else if(n->parent!=NULL) {
				for(n=n->parent; n->sibling==NULL && n->parent!=NULL; n=n->parent);
				n = n->sibling;
			} else break;
		}
	}
	return 0;
}

int q_tree_is_first_child(QTreeNode tree,QTreeNode node) {
	if(node==NULL) return 0;
	if(node->parent==NULL) return (tree!=NULL && tree==node);
	return (node->parent->child==node);
}

QTreeNode q_tree_parent(QTreeNode tree,QTreeNode node) {
	return node!=NULL? node->parent : NULL;
}

QTreeNode q_tree_first_sibling(QTreeNode tree,QTreeNode node) {
	if(node==NULL) return NULL;
	if(node->parent==NULL) return tree;
	return node->parent->child;
}

QTreeNode q_tree_previous(QTreeNode tree,QTreeNode node) {
	if(node==NULL) return NULL;
	if(node->parent!=NULL) tree = node->parent->child;
	if(tree==node) return NULL;
	while(tree!=NULL && tree->sibling!=node) tree = tree->sibling;
	return tree;
}

QTreeNode q_tree_next(QTreeNode tree,QTreeNode node) {
	return node!=NULL? node->sibling : NULL;
}

int q_tree_each(QTreeNode tree,QTreeNode *iter) {
	if(iter!=NULL && (*iter!=NULL || tree!=NULL)) {
		QTreeNode node = *iter;
		if(node==NULL) *iter = tree;
		else if(node->child!=NULL) *iter = node->child;
		else if(node->sibling!=NULL) *iter = node->sibling;
		else if(node->parent!=NULL) {
			for(node=node->parent; node->sibling==NULL && node->parent!=NULL; node=node->parent);
			*iter = node->sibling;
		} else *iter = NULL;
		return (*iter!=NULL);
	}
	return 0;
}

void q_tree_foreach(QTreeNode tree,void (*f)(QTreeNode,void *),void *data) {
	QTreeNode node = tree;
	if(f==NULL) return;
	while(node!=NULL) {
		(*f)(node,data);
		if(node->child!=NULL) node = node->child;
		else if(node->sibling!=NULL) node = node->sibling;
		else if(node->parent!=NULL) {
			for(node=node->parent; node->sibling==NULL && node->parent!=NULL; node=node->parent);
			node = node->sibling;
		} else break;
	}
}


