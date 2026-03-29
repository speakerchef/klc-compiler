$$
NodeProgram = \begin{cases}\
|\text{Scope}| \\
\end{cases}\\
$$
$$
Scope = \begin{cases}\
|\text{SyntaxNodes}| \\
|\text{Variable Lookup Table}| \\
\end{cases}\\
$$
$$
SyntaxNode = \begin{cases}\
|\text{Expr}| \\[3pt]
|\text{Identifier}| \\[3pt]
|\text{VarDeclaration}| \\[3pt]
|\text{Literal}| \\[3pt]
|\text{Stmt}| \\[3pt]
\end{cases}
$$
$$
Stmt = \begin{cases}\ 
\text{exit}(|Expr|)\\[3pt]
\text{let}(|Expr|)\\[3pt]
\text{mut}(|Expr|)\\[3pt]
\text{if}(|Expr|) \{|\text{Scope}|\}\\[3pt]
\end{cases}
$$
$$
Expr = \begin{cases}\
\text{atom}(|Literal|\text{OR}|Identifier|)\\[3pt]
\text{op}(|BinaryOperator|)\\[3pt]
\text{lhs}(|Expr|)\\[3pt]
\text{rhs}(|Expr|)\\[3pt]
\end{cases}
$$
$$
Identifier = \begin{cases}\
\text{Name} \\
\end{cases}
$$
$$
VarDeclaration = \begin{cases}\
\text{Kind} (\text{let}|\text{mut}) \\
|\text{Identifier}| \\
\text{Value} (|\text{SyntaxNode}|) \\
\end{cases}
$$
$$
Literal = \begin{cases}\
\text{Value}(float|int|string) \\
\end{cases}
$$

