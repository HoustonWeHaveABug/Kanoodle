#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define FREE(p) free(p), p = NULL

struct piece_s {
	unsigned long number;
	unsigned long rows;
	unsigned long row_slots;
	unsigned long columns1;
	unsigned long columns2;
	unsigned long column_slots;
	unsigned long cells_n;
	unsigned long blocks_n;
	char *cells;
	char *blocks;
	unsigned long *node_blocks;
	unsigned long *grid_blocks;
	unsigned long row_nodes_n;
};
typedef struct piece_s piece_t;

typedef struct node_s node_t;

struct row_node_s {
	unsigned long grid_origin;
	piece_t *piece;
	node_t *column;
};
typedef struct row_node_s row_node_t;

struct node_s {
	union {
		unsigned long rows;
		row_node_t *row_node;
	};
	node_t *left;
	node_t *right;
	node_t *top;
	node_t *bottom;
};

char *read_piece(piece_t *, unsigned long);
char *set_piece(piece_t *, unsigned long, unsigned long, unsigned long, unsigned long);
int isblock(int);
char *set_piece_blocks(piece_t *);
void free_data(unsigned long);
void free_piece(piece_t *);
char *rotate_piece(piece_t *, piece_t *);
char *flip_piece(piece_t *, piece_t *);
int compare_pieces(piece_t *, piece_t *);
void set_column_node(node_t *, node_t *);
void link_left(node_t *, node_t *);
void print_piece(piece_t *);
void set_piece_row_nodes(piece_t *);
void set_slot_row_nodes(piece_t *, unsigned long, unsigned long);
void set_row_node(unsigned long, piece_t *, unsigned long, node_t *);
void link_top(node_t *, node_t *);
void dlx_search(void);
void add_piece(piece_t *, unsigned long);
void cover_column(node_t *);
void uncover_column(node_t *);

static unsigned long grid_rows, grid_columns1, grid_columns2, grid_cells_n1, cost, solutions;

static char *grid_cells;
static piece_t *pieces;
static row_node_t *row_nodes;
static node_t *nodes, **tops, *header, *row_node;

int main(void) {
int r;
unsigned long grid_cells_n2, pieces_n, pieces_max, column_nodes_n1, column_nodes_n2, row_nodes_n, pieces_r, piece_f, piece_l, nodes_n, i, j, k;
	scanf("%lu", &grid_rows);
	if (!grid_rows) {
		return EXIT_FAILURE;
	}
	scanf("%lu", &grid_columns1);
	if (!grid_columns1) {
		return EXIT_FAILURE;
	}
	grid_cells_n1 = grid_rows*grid_columns1;
	grid_columns2 = grid_columns1+1;
	grid_cells_n2 = grid_rows*grid_columns2;
	grid_cells = malloc(grid_cells_n2+1);
	if (!grid_cells) {
		free_data(0UL);
		return EXIT_FAILURE;
	}
	for (i = grid_columns1; i < grid_cells_n2; i += grid_columns2) {
		grid_cells[i] = '\n';
	}
	grid_cells[grid_cells_n2] = 0;
	scanf("%lu", &pieces_n);
	if (!pieces_n) {
		return EXIT_FAILURE;
	}
	pieces_max = pieces_n*8;
	pieces = malloc(sizeof(piece_t)*pieces_max);
	if (!pieces) {
		return EXIT_FAILURE;
	}
	column_nodes_n1 = grid_cells_n1+pieces_n;
	column_nodes_n2 = column_nodes_n1+1;
	row_nodes_n = 0;
	pieces_r = 0;
	for (i = 0; i < pieces_n; i++) {
		if (!read_piece(&pieces[pieces_r], i)) {
			free_data(pieces_r);
			return EXIT_FAILURE;
		}
		piece_f = pieces_r;
		row_nodes_n += pieces[pieces_r].row_nodes_n;
		pieces_r++;
		j = 1;
		do {
			if (!rotate_piece(&pieces[pieces_r-1], &pieces[pieces_r])) {
				free_data(pieces_r);
				return EXIT_FAILURE;
			}
			r = compare_pieces(&pieces[piece_f], &pieces[pieces_r]);
			for (k = piece_f+1; k < pieces_r && !r; k++) {
				r = compare_pieces(&pieces[k], &pieces[pieces_r]);
			}
			if (!r) {
				row_nodes_n += pieces[pieces_r].row_nodes_n;
				pieces_r++;
				j++;
			}
			else {
				free_piece(&pieces[pieces_r]);
			}
		}
		while (j < 4 && !r);
		piece_l = pieces_r;
		j = piece_f;
		do {
			if (!flip_piece(&pieces[j], &pieces[pieces_r])) {
				free_data(pieces_r);
				return EXIT_FAILURE;
			}
			r = compare_pieces(&pieces[piece_f], &pieces[pieces_r]);
			for (k = piece_f+1; k < piece_l && !r; k++) {
				r = compare_pieces(&pieces[k], &pieces[pieces_r]);
			}
			if (!r) {
				row_nodes_n += pieces[pieces_r].row_nodes_n;
				pieces_r++;
				j++;
			}
			else {
				free_piece(&pieces[pieces_r]);
			}
		}
		while (j < piece_l && !r);
	}
	row_nodes = malloc(sizeof(row_node_t)*row_nodes_n);
	if (!row_nodes) {
		free_data(pieces_r);
		return EXIT_FAILURE;
	}
	nodes_n = column_nodes_n2+row_nodes_n;
	nodes = malloc(sizeof(node_t)*nodes_n);
	if (!nodes) {
		free_data(pieces_r);
		return EXIT_FAILURE;
	}
	for (i = column_nodes_n2; i < nodes_n; i++) {
		nodes[i].row_node = &row_nodes[i-column_nodes_n2];
	}
	tops = malloc(sizeof(node_t *)*column_nodes_n1);
	if (!tops) {
		free_data(pieces_r);
		return EXIT_FAILURE;
	}
	header = &nodes[column_nodes_n1];
	set_column_node(nodes, header);
	for (i = 0; i < column_nodes_n1; i++) {
		set_column_node(&nodes[i+1], &nodes[i]);
		tops[i] = &nodes[i];
	}
	row_node = header+1;
	for (i = 0; i < pieces_r; i++) {
		print_piece(&pieces[i]);
		set_piece_row_nodes(&pieces[i]);
	}
	for (i = 0; i < column_nodes_n1; i++) {
		link_top(&nodes[i], tops[i]);
	}
	dlx_search();
	printf("\nCost %lu\nSolutions %lu\n", cost, solutions);
	free_data(pieces_r);
	return EXIT_SUCCESS;
}

char *read_piece(piece_t *piece, unsigned long number) {
int c;
unsigned long rows, columns, cell, i, j;
	scanf("%lu", &rows);
	if (!rows) {
		return NULL;
	}
	scanf("%lu", &columns);
	if (!columns) {
		return NULL;
	}
	while (fgetc(stdin) != '\n');
	if (!set_piece(piece, number, rows, columns, 0UL)) {
		return NULL;
	}
	cell = 0;
	for (i = 0; i < piece->rows; i++) {
		j = 0;
		do {
			c = fgetc(stdin);
			if (c == '\n') {
				break;
			}
			else if (isblock(c)) {
				piece->blocks_n++;
				piece->cells[cell] = (char)c;
			}
			else {
				piece->cells[cell] = ' ';
			}
			cell++;
			j++;
		}
		while (j < piece->columns1);
		if (j < piece->columns1) {
			while (j < piece->columns1) {
				piece->cells[cell++] = ' ';
				j++;
			}
		}
		else {
			while (fgetc(stdin) != '\n');
		}
		piece->cells[cell++] = '\n';
	}
	piece->cells[cell] = 0;
	if (!piece->blocks_n || !set_piece_blocks(piece)) {
		FREE(piece->cells);
	}
	return piece->cells;
}

char *set_piece(piece_t *piece, unsigned long number, unsigned long rows, unsigned long columns, unsigned long blocks_n) {
	piece->number = number;
	piece->rows = rows;
	piece->row_slots = rows > grid_rows ? 0:grid_rows-rows+1;
	piece->columns1 = columns;
	piece->columns2 = columns+1;
	piece->column_slots = columns > grid_columns1 ? 0:grid_columns1-columns+1;
	piece->cells_n = rows*piece->columns2;
	piece->blocks_n = blocks_n;
	piece->cells = malloc(piece->cells_n+1);
	return piece->cells;
}

int isblock(int c) {
	return isprint(c) && !isspace(c);
}

char *set_piece_blocks(piece_t *piece) {
unsigned long block, i, j;
	piece->blocks = malloc(piece->blocks_n);
	if (!piece->blocks) {
		return NULL;
	}
	piece->node_blocks = malloc(sizeof(unsigned long)*piece->blocks_n);
	if (!piece->node_blocks) {
		FREE(piece->blocks);
		return NULL;
	}
	piece->grid_blocks = malloc(sizeof(unsigned long)*piece->blocks_n);
	if (!piece->grid_blocks) {
		FREE(piece->node_blocks);
		FREE(piece->blocks);
		return NULL;
	}
	block = 0;
	for (i = 0; i < piece->rows; i++) {
		for (j = 0; j < piece->columns1; j++) {
			if (piece->cells[i*piece->columns2+j] != ' ') {
				piece->blocks[block] = piece->cells[i*piece->columns2+j];
				piece->node_blocks[block] = i*grid_columns1+j;
				piece->grid_blocks[block] = i*grid_columns2+j;
				block++;
			}
		}
	}
	piece->row_nodes_n = piece->row_slots*piece->column_slots*(1+piece->blocks_n);
	return piece->blocks;
}

void free_data(unsigned long pieces_r) {
unsigned long i;
	if (tops) {
		FREE(tops);
	}
	if (nodes) {
		FREE(nodes);
	}
	if (row_nodes) {
		FREE(row_nodes);
	}
	if (pieces) {
		for (i = 0; i < pieces_r; i++) {
			free_piece(&pieces[i]);
		}
		FREE(pieces);
	}
	if (grid_cells) {
		FREE(grid_cells);
	}
}

void free_piece(piece_t *piece) {
	FREE(piece->grid_blocks);
	FREE(piece->node_blocks);
	FREE(piece->blocks);
	FREE(piece->cells);
}

char *rotate_piece(piece_t *piece, piece_t *rotated) {
unsigned long i, j;
	if (!set_piece(rotated, piece->number, piece->columns1, piece->rows, piece->blocks_n)) {
		return NULL;
	}
	for (i = 0; i < rotated->rows; i++) {
		for (j = 0; j < rotated->columns1; j++) {
			rotated->cells[i*rotated->columns2+j] = piece->cells[piece->cells_n-piece->columns2-j*piece->columns2+i];
		}
		rotated->cells[i*rotated->columns2+j] = '\n';
	}
	rotated->cells[rotated->cells_n] = 0;
	if (!set_piece_blocks(rotated)) {
		FREE(rotated->cells);
	}
	return rotated->cells;
}

char *flip_piece(piece_t *piece, piece_t *flipped) {
unsigned long i, j;
	if (!set_piece(flipped, piece->number, piece->rows, piece->columns1, piece->blocks_n)) {
		return NULL;
	}
	for (i = 0; i < flipped->cells_n; i += flipped->columns2) {
		for (j = 0; j < flipped->columns2; j++) {
			flipped->cells[i+j] = piece->cells[flipped->cells_n-flipped->columns2-i+j];
		}
	}
	flipped->cells[flipped->cells_n] = 0;
	if (!set_piece_blocks(flipped)) {
		FREE(flipped->cells);
	}
	return flipped->cells;
}

int compare_pieces(piece_t *piece1, piece_t *piece2) {
	return piece2->rows == piece1->rows && piece2->columns1 == piece1->columns1 && !strcmp(piece2->cells, piece1->cells);
}

void set_column_node(node_t *node, node_t *left) {
	node->rows = 0;
	link_left(node, left);
}

void link_left(node_t *node, node_t *left) {
	node->left = left;
	left->right = node;
}

void print_piece(piece_t *piece) {
	printf("\nPiece %lu\nSize %lux%lu\n", piece->number+1, piece->rows, piece->columns1);
	printf("%s", piece->cells);
}

void set_piece_row_nodes(piece_t *piece) {
unsigned long i, j;
	for (i = 0; i < piece->row_slots; i++) {
		for (j = 0; j < piece->column_slots; j++) {
			set_slot_row_nodes(piece, i*grid_columns1+j, i*grid_columns2+j);
		}
	}
}

void set_slot_row_nodes(piece_t *piece, unsigned long node_origin, unsigned long grid_origin) {
unsigned long i;
	set_row_node(grid_origin, piece, node_origin+piece->node_blocks[0], row_node+piece->blocks_n);
	for (i = 1; i < piece->blocks_n; i++) {
		set_row_node(grid_origin, piece, node_origin+piece->node_blocks[i], row_node-1);
	}
	set_row_node(grid_origin, piece, grid_cells_n1+piece->number, row_node-1);
}

void set_row_node(unsigned long grid_origin, piece_t *piece, unsigned long column, node_t *left) {
	row_node->row_node->grid_origin = grid_origin;
	row_node->row_node->piece = piece;
	row_node->row_node->column = &nodes[column];
	link_left(row_node, left);
	link_top(row_node, tops[column]);
	tops[column] = row_node++;
	nodes[column].rows++;
}

void link_top(node_t *node, node_t *top) {
	node->top = top;
	top->bottom = node;
}

void dlx_search(void) {
node_t *column_min, *column, *row, *node;
	cost++;
	if (header->right == header) {
		solutions++;
		printf("\n%s", grid_cells);
	}
	else {
		column_min = header->right;
		for (column = column_min->right; column != header; column = column->right) {
			if (column->rows < column_min->rows) {
				column_min = column;
			}
		}
		cover_column(column_min);
		for (row = column_min->bottom; row != column_min; row = row->bottom) {
			add_piece(row->row_node->piece, row->row_node->grid_origin);
			for (node = row->right; node != row; node = node->right) {
				cover_column(node->row_node->column);
			}
			dlx_search();
			for (node = row->left; node != row; node = node->left) {
				uncover_column(node->row_node->column);
			}
		}
		uncover_column(column_min);
	}
}

void add_piece(piece_t *piece, unsigned long grid_origin) {
unsigned long i;
	for (i = 0; i < piece->blocks_n; i++) {
		grid_cells[grid_origin+piece->grid_blocks[i]] = piece->blocks[i];
	}
}

void cover_column(node_t *column) {
node_t *row, *node;
	column->right->left = column->left;
	column->left->right = column->right;
	for (row = column->bottom; row != column; row = row->bottom) {
		for (node = row->right; node != row; node = node->right) {
			node->row_node->column->rows--;
			node->bottom->top = node->top;
			node->top->bottom = node->bottom;
		}
	}
}

void uncover_column(node_t *column) {
node_t *row, *node;
	for (row = column->top; row != column; row = row->top) {
		for (node = row->left; node != row; node = node->left) {
			node->top->bottom = node->bottom->top = node;
			node->row_node->column->rows++;
		}
	}
	column->left->right = column->right->left = column;
}
