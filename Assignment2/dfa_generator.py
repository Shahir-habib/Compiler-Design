from flask import Flask, request, jsonify
from flask_cors import CORS
import re

app = Flask(__name__)
CORS(app)

class State:
    def __init__(self, is_accepting=False):
        self.transitions = {}  # Key: symbol (str) or None for epsilon, Value: set of States
        self.is_accepting = is_accepting

class NFA:
    def __init__(self, start, end):
        self.start = start
        self.end = end  # End state

def epsilon_closure(state):
    closure = set()
    stack = [state]
    closure.add(state)
    while stack:
        s = stack.pop()
        for eps_trans in s.transitions.get(None, set()):
            if eps_trans not in closure:
                closure.add(eps_trans)
                stack.append(eps_trans)
    return closure

def move(states, symbol):
    next_states = set()
    for state in states:
        if symbol in state.transitions:
            next_states.update(state.transitions[symbol])
    return next_states

def insert_concat_operators(regex):
    if len(regex) == 0:
        return regex
    new_regex = [regex[0]]
    for i in range(1, len(regex)):
        prev = regex[i - 1]
        curr = regex[i]
        # Handle character classes and escaped characters
        if curr == '[' or (prev == '\\' and curr in {'d', 's', 'w', 'b', 'D', 'S', 'W'}):
            new_regex.append(curr)
            continue
        # Insert concatenation operator where necessary
        if (prev in [')', '*', '?', '+'] or (prev not in {'|', '(', ')', '*', '+', '?'})) and \
           (curr == '(' or (curr not in {'|', ')', '*', '+', '?'})):
            new_regex.append('.')
        new_regex.append(curr)
    return ''.join(new_regex)

def shunting_yard(regex):
    precedence = {'*': 4, '?': 4, '+': 4, '.': 3, '|': 2, '(': 1}
    output = []
    stack = []
    i = 0
    while i < len(regex):
        token = regex[i]
        if token == '\\':
            # Handle escaped characters like \d, \s, \w, \b, etc.
            if i + 1 >= len(regex):
                raise ValueError("Invalid escape sequence")
            output.append(regex[i:i+2])
            i += 2
            continue
        elif token == '[':
            # Handle character classes like [a-z], [0-9_], etc.
            j = i + 1
            while j < len(regex) and regex[j] != ']':
                j += 1
            if j >= len(regex):
                raise ValueError("Unclosed character class")
            output.append(regex[i:j+1])
            i = j + 1
            continue
        elif token == '(':
            stack.append(token)
        elif token == ')':
            while stack and stack[-1] != '(':
                output.append(stack.pop())
            if not stack:
                raise ValueError("Mismatched parentheses")
            stack.pop()  # Pop the '('
        elif token in precedence:
            while stack and stack[-1] != '(' and precedence[stack[-1]] >= precedence[token]:
                output.append(stack.pop())
            stack.append(token)
        else:
            output.append(token)
        i += 1
    while stack:
        if stack[-1] == '(':
            raise ValueError("Mismatched parentheses")
        output.append(stack.pop())
    return ''.join(output)

def build_nfa(postfix):
    stack = []
    for token in postfix:
        if token == '*':
            nfa = stack.pop()
            new_start = State()
            new_end = State(is_accepting=True)
            new_start.transitions[None] = {nfa.start, new_end}
            nfa.end.is_accepting = False
            nfa.end.transitions[None] = {nfa.start, new_end}
            stack.append(NFA(new_start, new_end))
        elif token == '.':
            nfa2 = stack.pop()
            nfa1 = stack.pop()
            nfa1.end.is_accepting = False
            nfa1.end.transitions[None] = {nfa2.start}
            stack.append(NFA(nfa1.start, nfa2.end))
        elif token == '|':
            nfa2 = stack.pop()
            nfa1 = stack.pop()
            new_start = State()
            new_end = State(is_accepting=True)
            new_start.transitions[None] = {nfa1.start, nfa2.start}
            nfa1.end.is_accepting = False
            nfa2.end.is_accepting = False
            nfa1.end.transitions[None] = {new_end}
            nfa2.end.transitions[None] = {new_end}
            stack.append(NFA(new_start, new_end))
        elif token == '?':
            nfa = stack.pop()
            new_start = State()
            new_end = State(is_accepting=True)
            new_start.transitions[None] = {nfa.start, new_end}
            nfa.end.is_accepting = False
            nfa.end.transitions[None] = {new_end}
            stack.append(NFA(new_start, new_end))
        elif token == '+':
            nfa = stack.pop()
            new_start = State()
            new_end = State(is_accepting=True)
            new_start.transitions[None] = {nfa.start}
            nfa.end.is_accepting = False
            nfa.end.transitions[None] = {new_end, nfa.start}
            stack.append(NFA(new_start, new_end))
        elif token.startswith('[') and token.endswith(']'):
            # Handle character classes like [a-z], [0-9_], etc.
            char_class = token[1:-1]
            start = State()
            end = State(is_accepting=True)
            if '-' in char_class:
                # Handle ranges like a-z, 0-9
                ranges = char_class.split(',')
                for r in ranges:
                    if '-' in r:
                        start_char, end_char = r.split('-')
                        for c in range(ord(start_char), ord(end_char) + 1):
                            start.transitions[chr(c)] = {end}
                    else:
                        start.transitions[r] = {end}
            else:
                for c in char_class:
                    start.transitions[c] = {end}
            stack.append(NFA(start, end))
        elif token.startswith('\\'):
            # Handle escaped characters like \d, \s, \w, \b, etc.
            escape_char = token[1]
            start = State()
            end = State(is_accepting=True)
            if escape_char == 'd':
                # \d matches any digit
                for c in range(ord('0'), ord('9') + 1):
                    start.transitions[chr(c)] = {end}
            elif escape_char == 's':
                # \s matches any whitespace character
                for c in [' ', '\t', '\n', '\r']:
                    start.transitions[c] = {end}
            elif escape_char == 'w':
                # \w matches any word character (alphanumeric + underscore)
                for c in range(ord('0'), ord('9') + 1):
                    start.transitions[chr(c)] = {end}
                for c in range(ord('A'), ord('Z') + 1):
                    start.transitions[chr(c)] = {end}
                for c in range(ord('a'), ord('z') + 1):
                    start.transitions[chr(c)] = {end}
                start.transitions['_'] = {end}
            elif escape_char == 'b':
                # \b matches a word boundary 
                pass
            else:
                # Handle other escaped characters
                start.transitions[escape_char] = {end}
            stack.append(NFA(start, end))
        else:
            # Handle single characters
            start = State()
            end = State(is_accepting=True)
            start.transitions[token] = {end}
            stack.append(NFA(start, end))
    return stack.pop()

def nfa_to_dfa(nfa, alphabet):
    initial = frozenset(epsilon_closure(nfa.start))
    dfa_states = [initial]
    dfa_transitions = {}
    state_map = {initial: 0}
    accepting_states = []
    if any(s.is_accepting for s in initial):
        accepting_states.append(0)
    queue = [initial]
    state_counter = 1

    while queue:
        current = queue.pop(0)
        current_idx = state_map[current]

        for symbol in alphabet:
            moved = move(current, symbol)
            closure = set()
            for s in moved:
                closure.update(epsilon_closure(s))
            closure_frozen = frozenset(closure)
            if not closure_frozen:
                continue
            if closure_frozen not in state_map:
                state_map[closure_frozen] = state_counter
                dfa_states.append(closure_frozen)
                if any(s.is_accepting for s in closure_frozen):
                    accepting_states.append(state_counter)
                queue.append(closure_frozen)
                state_counter += 1
            dfa_transitions[(current_idx, symbol)] = state_map[closure_frozen]

    # Handle dead state
    dead_state = None
    all_states = list(state_map.values())
    for state_idx in all_states.copy():
        for symbol in alphabet:
            if (state_idx, symbol) not in dfa_transitions:
                if dead_state is None:
                    dead_state = state_counter
                    state_counter += 1
                    for s in alphabet:
                        dfa_transitions[(dead_state, s)] = dead_state
                dfa_transitions[(state_idx, symbol)] = dead_state
    if dead_state is not None:
        for symbol in alphabet:
            dfa_transitions.setdefault((dead_state, symbol), dead_state)

    # Build DFA table
    dfa_table = {}
    max_state = state_counter if dead_state is not None else state_counter
    for state in range(max_state):
        dfa_table[state] = {}
        for symbol in alphabet:
            dfa_table[state][symbol] = dfa_transitions.get((state, symbol), dead_state)

    return dfa_table, accepting_states

def validate_regex(regex, alphabet):
    valid_symbols = set(alphabet) | {'|', '*', '+', '?', '(', ')', '[', ']', '\\', '.'}
    stack = []
    i = 0
    while i < len(regex):
        c = regex[i]
        if c == '\\':
            # Handle escaped characters
            if i + 1 >= len(regex):
                return False, "Invalid escape sequence"
            next_char = regex[i + 1]
            if next_char not in {'d', 's', 'w', 'b', 'D', 'S', 'W', '\\', '.', '|', '*', '+', '?', '(', ')', '[', ']'}:
                return False, f"Invalid escape sequence '\\{next_char}'"
            i += 2
        elif c == '[':
            # Handle character classes
            j = i + 1
            while j < len(regex) and regex[j] != ']':
                j += 1
            if j >= len(regex):
                return False, "Unclosed character class"
            i = j + 1
        elif c == '(':
            stack.append(c)
            i += 1
        elif c == ')':
            if not stack:
                return False, "Unbalanced parentheses"
            stack.pop()
            i += 1
        elif c not in valid_symbols:
            return False, f"Invalid character '{c}' in regular expression"
        else:
            i += 1
    if stack:
        return False, "Unbalanced parentheses"
    return True, ""
from graphviz import Digraph
# def generate_dfa_diagram(dfa_table, accepting_states, alphabet, output_file='dfa'):
#     # Create a directed graph
#     dot = Digraph(comment='DFA State Transition Diagram')

#     # Add states
#     for state in dfa_table:
#         if state in accepting_states:
#             # Double circle for accepting states
#             dot.node(f'q{state}', shape='doublecircle')
#         else:
#             # Single circle for non-accepting states
#             dot.node(f'q{state}', shape='circle')

#     # Add transitions
#     for state, transitions in dfa_table.items():
#         for symbol, next_state in transitions.items():
#             dot.edge(f'q{state}', f'q{next_state}', label=symbol)

#     # Render the graph to a file
#     dot.render(output_file, format='png', cleanup=False)
def generate_dfa_diagram(dfa_table, accepting_states, alphabet, output_file='dfa'):
  
    dot = Digraph(comment='Colorful DFA State Transition Diagram')

    state_colors = ['lightcoral', 'lightblue', 'lightgreen', 'plum', 'lightsalmon', 'lightgoldenrodyellow']
    edge_colors = ['red', 'blue', 'green', 'purple', 'orange', 'gold']

    for i, state in enumerate(dfa_table):
        color = state_colors[i % len(state_colors)]
        if state in accepting_states:
            
            dot.node(f'q{state}', shape='doublecircle', style='filled', fillcolor=color)
        else:
            
            dot.node(f'q{state}', shape='circle', style='filled', fillcolor=color)

    # Add transitions with different colors and styles
    for state, transitions in dfa_table.items():
        for symbol, next_state in transitions.items():
            edge_color = edge_colors[hash(symbol) % len(edge_colors)]
            dot.edge(f'q{state}', f'q{next_state}', label=symbol, color=edge_color)

    # Render the graph to a file
    dot.render(output_file, format='png')
    
@app.route('/generate-dfa', methods=['POST'])
def generate_dfa():
    data = request.json
    alphabet = data['alphabet']
    regex = data['regex']

    is_valid, msg = validate_regex(regex, alphabet)
    if not is_valid:
        return jsonify({"error": msg}), 400

    modified_regex = insert_concat_operators(regex)
    try:
        postfix = shunting_yard(modified_regex)
    except ValueError as e:
        return jsonify({"error": str(e)}), 400

    try:
        nfa = build_nfa(postfix)
    except Exception as e:
        return jsonify({"error": str(e)}), 400

    dfa_table, accepting_states = nfa_to_dfa(nfa, alphabet)

    # Generate the DFA diagram
    generate_dfa_diagram(dfa_table, accepting_states, alphabet)

    return jsonify({
        "dfa_table": dfa_table,
        "accepting_states": accepting_states,
        "diagram_url": "./dfa.png"  # Serve the diagram image
    })

if __name__ == "__main__":
    app.run(debug=True)