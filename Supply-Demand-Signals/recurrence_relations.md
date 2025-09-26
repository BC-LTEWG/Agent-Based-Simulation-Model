# Firm–Distributor Recurrence System (Labor-Time Certificates)

This document specifies a discrete-time model linking **Firms** and **Distributors**.  
We currently **ignore fixed capital `f`** and track:

- `c` = raw materials inventory  
- `l` = labor-time certificates  

---

## Notation

- Firm state: $(F_c(i), F_l(i))$  
- Distributor state: $(D_c(i), D_l(i))$  

**Indicator function:**

$$
\mathbf{1}[\text{statement}] =
\begin{cases}
1, & \text{if the statement is true} \\
0, & \text{otherwise}
\end{cases}
$$

- $a_c > 0$: raw materials required per unit of output  
- $a_l > 0$: labor-hours required per unit of output  
- $b > 0$: shipment size when the firm restocks  

---

## Production in Cycle $i$

Output (goods produced):

$$
u_i = \min\left(\frac{F_c(i)}{a_c}, \frac{F_l(i)}{a_l}\right)
$$

- **Materials consumed:**  
$$
M_i = a_c \cdot u_i
$$

- **Labor consumed:**  
$$
L_i = a_l \cdot u_i
$$

---

## Trade/Payment Rules

- **Shipment when firm is empty:**

$$
S_i = \min\{b, D_c(i)\} \cdot \mathbf{1}[F_c(i)=0]
$$

- **Payment in labor certificates (value of goods produced):**  
  Each good embodies $a_l$ labor-hours. Producing $u_i$ goods requires $a_l \cdot u_i$ hours.  
  Therefore:

$$
P_i = a_l \cdot u_i
$$

---

## Recurrence Relations

### Firm

$$
F_c(i+1) = F_c(i) - a_c \cdot u_i + S_i
$$

$$
F_l(i+1) = F_l(i) - a_l \cdot u_i
$$

### Distributor

$$
D_c(i+1) = D_c(i) - S_i + u_i
$$

$$
D_l(i+1) = D_l(i) + P_i = D_l(i) + a_l \cdot u_i
$$

---

## Example Initialization

At cycle $i=0$:

- Firm: $F_c(0)=0$, $F_l(0)=100$  
- Distributor: $D_c(0)=100$, $D_l(0)=100$  
- Coefficients: $a_c=1$, $a_l=4$, $b=50$

This means **each good requires 1 unit of raw material and 4 labor-hours**.  
Producing $u_i$ goods pays $4u_i$ labor-time certificates to the distributor.

---

## Flow Diagram

```mermaid
flowchart LR
  F[Firm: (F_c, F_l)] ---|produces u_i| D[Distributor: (D_c, D_l)]
  D -- "ships S_i when F_c=0" --> F
  F -- "pays P_i = a_l * u_i" --> D

  subgraph Cycle
    direction LR
    F -->|"consumes a_c*u_i and a_l*u_i"| F
    D -->|"receives u_i; sends S_i"| D
  end
```