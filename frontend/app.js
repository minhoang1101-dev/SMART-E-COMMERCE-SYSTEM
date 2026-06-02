const state = {
  user: null,
  products: [],
  cart: JSON.parse(localStorage.getItem("smartCart") || "{}")
};

const elements = {
  authPanel: document.querySelector("#authPanel"),
  authTabs: document.querySelectorAll("[data-auth-mode]"),
  authForms: document.querySelectorAll("[data-auth-panel]"),
  sessionText: document.querySelector("#sessionText"),
  logoutButton: document.querySelector("#logoutButton"),
  loginForm: document.querySelector("#loginForm"),
  username: document.querySelector("#username"),
  password: document.querySelector("#password"),
  registerForm: document.querySelector("#registerForm"),
  registerUsername: document.querySelector("#registerUsername"),
  registerPassword: document.querySelector("#registerPassword"),
  guestButton: document.querySelector("#guestButton"),
  adminPanel: document.querySelector("#adminPanel"),
  productForm: document.querySelector("#productForm"),
  productName: document.querySelector("#productName"),
  productCategory: document.querySelector("#productCategory"),
  productPrice: document.querySelector("#productPrice"),
  searchInput: document.querySelector("#searchInput"),
  productList: document.querySelector("#productList"),
  cartList: document.querySelector("#cartList"),
  cartTotal: document.querySelector("#cartTotal"),
  clearCartButton: document.querySelector("#clearCartButton"),
  checkoutButton: document.querySelector("#checkoutButton"),
  recommendationList: document.querySelector("#recommendationList"),
  orderList: document.querySelector("#orderList"),
  toast: document.querySelector("#toast")
};

function setAuthMode(mode) {
  elements.authTabs.forEach((tab) => {
    const active = tab.dataset.authMode === mode;
    tab.classList.toggle("active", active);
    tab.setAttribute("aria-selected", String(active));
  });

  elements.authForms.forEach((form) => {
    form.classList.toggle("hidden", form.dataset.authPanel !== mode);
  });
}

function money(value) {
  return new Intl.NumberFormat("en-US", {
    style: "currency",
    currency: "USD"
  }).format(Number(value || 0));
}

function saveCart() {
  localStorage.setItem("smartCart", JSON.stringify(state.cart));
}

function showToast(message) {
  elements.toast.textContent = message;
  elements.toast.classList.remove("hidden");
  window.clearTimeout(showToast.timer);
  showToast.timer = window.setTimeout(() => elements.toast.classList.add("hidden"), 2800);
}

async function api(path, options = {}) {
  const response = await fetch(path, {
    headers: { "Content-Type": "application/json", ...(options.headers || {}) },
    ...options
  });
  const data = await response.json();
  if (!response.ok) {
    throw new Error(data.error || "Request failed.");
  }
  return data;
}

function renderSession() {
  if (state.user) {
    elements.sessionText.textContent = `${state.user.username} (${state.user.role})`;
    elements.logoutButton.classList.remove("hidden");
    elements.authPanel.classList.add("hidden");
    elements.adminPanel.classList.toggle("hidden", state.user.role !== "admin");
  } else {
    elements.sessionText.textContent = "Guest";
    elements.logoutButton.classList.add("hidden");
    elements.authPanel.classList.remove("hidden");
    elements.adminPanel.classList.add("hidden");
  }
}

function renderProducts() {
  const search = elements.searchInput.value.trim().toLowerCase();
  const products = state.products.filter((product) => {
    return product.name.toLowerCase().includes(search) || product.category.toLowerCase().includes(search);
  });

  if (!products.length) {
    elements.productList.innerHTML = '<p class="empty">No products found.</p>';
    return;
  }

  elements.productList.innerHTML = products.map((product) => `
    <article class="product-card">
      <span class="category">${product.category}</span>
      <h3>${product.name}</h3>
      <div class="price">${money(product.price)}</div>
      <div class="stats">
        <span>${product.views} views</span>
        <span>${product.purchases} purchases</span>
      </div>
      <div class="actions">
        <button type="button" data-view="${product.id}" class="ghost">View</button>
        <button type="button" data-add="${product.id}">Add</button>
      </div>
      ${state.user?.role === "admin" ? `<button type="button" class="danger" data-delete="${product.id}">Delete</button>` : ""}
    </article>
  `).join("");
}

function renderCart() {
  const entries = Object.entries(state.cart)
    .map(([id, quantity]) => ({
      product: state.products.find((item) => item.id === Number(id)),
      quantity
    }))
    .filter((entry) => entry.product);

  if (!entries.length) {
    elements.cartList.innerHTML = '<p class="empty">Your cart is empty.</p>';
    elements.cartTotal.textContent = money(0);
    return;
  }

  elements.cartList.innerHTML = entries.map(({ product, quantity }) => `
    <div class="cart-item">
      <div>
        <strong>${product.name}</strong>
        <p class="meta">${money(product.price)} each</p>
      </div>
      <input type="number" min="1" value="${quantity}" data-quantity="${product.id}">
    </div>
  `).join("");

  const total = entries.reduce((sum, entry) => sum + entry.product.price * entry.quantity, 0);
  elements.cartTotal.textContent = money(total);
}

function renderRecommendations(recommendations) {
  if (!recommendations.length) {
    elements.recommendationList.innerHTML = '<p class="empty">No recommendations yet.</p>';
    return;
  }

  elements.recommendationList.innerHTML = recommendations.map((product) => `
    <div class="mini-item">
      <strong>${product.name}</strong>
      <p class="meta">${product.category} - ${money(product.price)}</p>
    </div>
  `).join("");
}

function renderOrders(orders) {
  if (!state.user) {
    elements.orderList.innerHTML = '<p class="empty">Sign in to see orders.</p>';
    return;
  }

  if (!orders.length) {
    elements.orderList.innerHTML = '<p class="empty">No orders yet.</p>';
    return;
  }

  elements.orderList.innerHTML = orders.map((order) => `
    <div class="mini-item">
      <strong>Order #${order.orderId}</strong>
      <p class="meta">${order.username} - ${money(order.totalPrice)}</p>
      <p class="meta">${new Date(order.timestamp).toLocaleString()}</p>
    </div>
  `).join("");
}

async function loadProducts() {
  const data = await api("/api/products");
  state.products = data.products;
  renderProducts();
  renderCart();
}

async function loadRecommendations() {
  const data = await api("/api/recommendations");
  renderRecommendations(data.recommendations);
}

async function loadOrders() {
  if (!state.user) {
    renderOrders([]);
    return;
  }

  const data = await api("/api/orders");
  renderOrders(data.orders);
}

async function refresh() {
  await loadProducts();
  await Promise.all([loadRecommendations(), loadOrders()]);
}

elements.authTabs.forEach((tab) => {
  tab.addEventListener("click", () => setAuthMode(tab.dataset.authMode));
});

elements.loginForm.addEventListener("submit", async (event) => {
  event.preventDefault();
  try {
    const data = await api("/api/login", {
      method: "POST",
      body: JSON.stringify({
        username: elements.username.value,
        password: elements.password.value
      })
    });
    state.user = data.user;
    renderSession();
    await loadOrders();
    showToast("Signed in.");
  } catch (error) {
    showToast(error.message);
  }
});

elements.registerForm.addEventListener("submit", async (event) => {
  event.preventDefault();
  try {
    const data = await api("/api/register", {
      method: "POST",
      body: JSON.stringify({
        username: elements.registerUsername.value,
        password: elements.registerPassword.value
      })
    });
    state.user = data.user;
    elements.registerForm.reset();
    renderSession();
    await loadOrders();
    showToast("Account created.");
  } catch (error) {
    showToast(error.message);
  }
});

elements.guestButton.addEventListener("click", async () => {
  try {
    const data = await api("/api/guest", { method: "POST" });
    state.user = data.user;
    renderSession();
    await loadOrders();
    showToast("Continuing as guest.");
  } catch (error) {
    showToast(error.message);
  }
});

elements.logoutButton.addEventListener("click", async () => {
  await api("/api/logout", { method: "POST" });
  state.user = null;
  setAuthMode("signin");
  renderSession();
  renderOrders([]);
  showToast("Signed out.");
});

elements.productForm.addEventListener("submit", async (event) => {
  event.preventDefault();
  try {
    await api("/api/products", {
      method: "POST",
      body: JSON.stringify({
        name: elements.productName.value,
        category: elements.productCategory.value,
        price: elements.productPrice.value
      })
    });
    elements.productForm.reset();
    await refresh();
    showToast("Product added.");
  } catch (error) {
    showToast(error.message);
  }
});

elements.searchInput.addEventListener("input", renderProducts);

elements.productList.addEventListener("click", async (event) => {
  const viewId = event.target.dataset.view;
  const addId = event.target.dataset.add;
  const deleteId = event.target.dataset.delete;

  try {
    if (viewId) {
      await api(`/api/products/${viewId}/view`, { method: "POST" });
      await refresh();
      showToast("Product view saved.");
    }

    if (addId) {
      state.cart[addId] = (state.cart[addId] || 0) + 1;
      saveCart();
      renderCart();
      showToast("Added to cart.");
    }

    if (deleteId) {
      await api(`/api/products/${deleteId}`, { method: "DELETE" });
      delete state.cart[deleteId];
      saveCart();
      await refresh();
      showToast("Product deleted.");
    }
  } catch (error) {
    showToast(error.message);
  }
});

elements.cartList.addEventListener("change", (event) => {
  const productId = event.target.dataset.quantity;
  if (!productId) return;
  const quantity = Math.max(1, Number(event.target.value || 1));
  state.cart[productId] = quantity;
  saveCart();
  renderCart();
});

elements.clearCartButton.addEventListener("click", () => {
  state.cart = {};
  saveCart();
  renderCart();
});

elements.checkoutButton.addEventListener("click", async () => {
  try {
    if (!state.user) {
      showToast("Sign in, sign up, or continue as guest to checkout.");
      return;
    }

    const items = Object.entries(state.cart).map(([productId, quantity]) => ({
      productId: Number(productId),
      quantity
    }));
    await api("/api/orders", {
      method: "POST",
      body: JSON.stringify({ items })
    });
    state.cart = {};
    saveCart();
    await refresh();
    showToast("Order created.");
  } catch (error) {
    showToast(error.message);
  }
});

async function init() {
  const data = await api("/api/me");
  state.user = data.user;
  renderSession();
  await refresh();
}

init().catch((error) => showToast(error.message));
