// This function adds a pointer to a new leaf search-tree node at the front of the frontier.
int add_to_frontier(struct frontier_node* node) {
    if (node == NULL) {
        return -1;
    }

    node->previous = NULL;
    node->next = head;

    if (head == NULL) {
        head = node;
        tail = node;
    } else {
        head->previous = node;
        head = node;
    }

    return 0;
}

// Check whether a vector is a complete assignment and it is also valid.
int solution(struct frontier_node* node) {
    for (int i = 0; i < N; i++) {
        if (node->vector[i] == 0) {
            return 0;
        }
    }

    return valid(node);
}

// Given a partial assignment vector, for which a subset of the first propositions have values, 
// this function pushes up to two new vectors to the frontier, which concern giving to the first unassigned 
// proposition the values true=1 and false=-1, after checking that the new vectors are valid.
void generate_children(struct frontier_node* node) {
    int i;
    int* vector = node->vector;

    // Find the first proposition with no assigned value.
    for (i = 0; i<N && vector[i] != 0; i++);

    vector[i] = -1;
    struct frontier_node *negative = (struct frontier_node*) malloc(sizeof(struct frontier_node));
    negative->vector = (int*)malloc(N * sizeof(int));
    if (negative == NULL || negative->vector == NULL) {
        mem_error = -1;
        return;
    }
    copy(vector, negative->vector);
    // Check whether a "false" assignment is acceptable...
    if (valid(negative)) {
        // ...and pushes it to the frontier.
        add_to_frontier(negative);
    }

    vector[i] = 1;
    struct frontier_node *positive = (struct frontier_node*) malloc(sizeof(struct frontier_node));
    positive->vector = (int*)malloc(N * sizeof(int));
    if (positive == NULL || positive->vector == NULL) {
        mem_error = -1;
        return;
    }
    copy(vector, positive->vector);
    // Check whether a "true" assignment is acceptable...
    if (valid(positive)) {
        // ...and pushes it to the frontier.
        add_to_frontier(positive);
    }
}

// This function implements the searching algorithm we've used,
// checking the frontier head if it's a solution, otherwise creating its
// children and pushes them to the frontier.
struct frontier_node* search() {
    struct frontier_node* current_node;
    struct frontier_node* temp_node;

    // Initializing the frontier.
    struct frontier_node* root = (struct frontier_node*) malloc(sizeof(struct frontier_node));
    root->vector = (int*)malloc(N * sizeof(int));
    if (root == NULL || root->vector == NULL) {
        mem_error = -1;
        return NULL;
    }
    for (int i = 0; i < N; i++) {
        root->vector[i] = 0;
    }

    generate_children(root);

    t1 = clock();

    // While the frontier is not empty...
    while (head != NULL) {
        // Extract the first node from the frontier.
        current_node = head;

        // If it is a solution return it.
        if (solution(current_node)) {
            t2 = clock();
            return current_node;
        }

        // Generate its children.
        generate_children(current_node);

        // Delete the first node of the frontier.
        temp_node = head;
        head = head->next;
        free(temp_node);
        if (head == NULL) {
            tail = NULL;
        } else {
            head->previous = NULL;
        }
    }

    t2 = clock();

    return NULL;
}
