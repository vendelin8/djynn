#ifndef _LIBQ_TREE_H_
#define _LIBQ_TREE_H_

/**
 * @file libq/tree.h
 * @author Per Löwgren
 * @date Modified: 2015-07-25
 * @date Created: 2015-07-24
 */

typedef struct _QTreeNode *QTreeNode;

struct _QTreeNode {
	QTreeNode parent;
	QTreeNode child;
	QTreeNode sibling;
};

int q_tree_remove(QTreeNode *tree,QTreeNode node);
int q_tree_insert_before(QTreeNode *tree,QTreeNode dest,QTreeNode node);
int q_tree_insert_after(QTreeNode *tree,QTreeNode dest,QTreeNode node);
int q_tree_insert_child(QTreeNode *tree,QTreeNode dest,QTreeNode node);

int q_tree_count(QTreeNode tree);
int q_tree_contains(QTreeNode tree,QTreeNode node);
int q_tree_is_first_child(QTreeNode tree,QTreeNode node);
QTreeNode q_tree_parent(QTreeNode tree,QTreeNode node);
QTreeNode q_tree_first_sibling(QTreeNode tree,QTreeNode node);
QTreeNode q_tree_previous(QTreeNode tree,QTreeNode node);
QTreeNode q_tree_next(QTreeNode tree,QTreeNode node);
int q_tree_each(QTreeNode tree,QTreeNode *iter);
void q_tree_foreach(QTreeNode tree,void (*f)(QTreeNode,void *),void *data);

#endif /* _LIBQ_TREE_H_ */

