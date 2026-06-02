const http = require("http");
const fs = require("fs/promises");
const path = require("path");
const crypto = require("crypto");

const rootDir = path.resolve(__dirname, "..");
const dataDir = path.join(rootDir, "data");
const publicDir = path.join(rootDir, "frontend");
const port = Number(process.env.PORT || 3000);

const sessions = new Map();

const files = {
  products: path.join(dataDir, "products.txt"),
  users: path.join(dataDir, "users.txt"),
  orders: path.join(dataDir, "orders.txt"),
  interactions: path.join(dataDir, "interactions.txt")
};

const contentTypes = {
  ".html": "text/html; charset=utf-8",
  ".css": "text/css; charset=utf-8",
  ".js": "application/javascript; charset=utf-8",
  ".json": "application/json; charset=utf-8",
  ".svg": "image/svg+xml"
};

async function ensureDataFiles() {
  await fs.mkdir(dataDir, { recursive: true });
  await Promise.all(Object.values(files).map(async (file) => {
    try {
      await fs.access(file);
    } catch {
      await fs.writeFile(file, "", "utf8");
    }
  }));
}

function splitLine(line, separator) {
  return line.split(separator).map((value) => value.trim());
}

async function readLines(file) {
  const content = await fs.readFile(file, "utf8");
  return content.split(/\r?\n/).map((line) => line.trim()).filter(Boolean);
}

async function writeLines(file, lines) {
  const body = lines.length ? `${lines.join("\n")}\n` : "";
  await fs.writeFile(file, body, "utf8");
}

function parseProduct(line) {
  const [id, name, category, price, views = "0", purchases = "0"] = splitLine(line, "|");
  return {
    id: Number(id),
    name,
    category,
    price: Number(price),
    views: Number(views),
    purchases: Number(purchases)
  };
}

function serializeProduct(product) {
  return [
    product.id,
    product.name,
    product.category,
    Number(product.price || 0).toFixed(6),
    product.views || 0,
    product.purchases || 0
  ].join("|");
}

async function getProducts() {
  return (await readLines(files.products)).map(parseProduct).filter((product) => product.id);
}

async function saveProducts(products) {
  await writeLines(files.products, products.map(serializeProduct));
}

function parseUser(line) {
  const [username, password, role] = splitLine(line, "|");
  return { username, password, role };
}

function serializeUser(user) {
  return [user.username, user.password, user.role].join("|");
}

async function getUsers() {
  return (await readLines(files.users)).map(parseUser).filter((user) => user.username);
}

async function saveUsers(users) {
  await writeLines(files.users, users.map(serializeUser));
}

function createSession(response, user) {
  const token = crypto.randomBytes(24).toString("hex");
  const sessionUser = { username: user.username, role: user.role };
  sessions.set(token, sessionUser);
  sendJson(response, 200, { user: sessionUser }, {
    "Set-Cookie": `session=${token}; HttpOnly; SameSite=Lax; Path=/`
  });
}

function parseInteraction(line) {
  const tokens = splitLine(line, "|");

  if (tokens.length >= 4 && /^\d+$/.test(tokens[1])) {
    return {
      username: tokens[0],
      productId: Number(tokens[1]),
      views: Number(tokens[2] || 0),
      purchases: Number(tokens[3] || 0)
    };
  }

  return null;
}

async function getInteractions() {
  return (await readLines(files.interactions))
    .map(parseInteraction)
    .filter(Boolean);
}

async function saveInteractions(interactions) {
  await writeLines(files.interactions, interactions.map((interaction) => [
    interaction.username,
    interaction.productId,
    interaction.views,
    interaction.purchases
  ].join("|")));
}

async function updateInteraction(username, productId, field, amount = 1) {
  if (!username) return;

  const interactions = await getInteractions();
  let interaction = interactions.find((item) => item.username === username && item.productId === productId);

  if (!interaction) {
    interaction = { username, productId, views: 0, purchases: 0 };
    interactions.push(interaction);
  }

  interaction[field] += amount;
  await saveInteractions(interactions);
}

function parseOrder(line) {
  const [orderId, username, totalPrice, timestamp, itemsText = ""] = splitLine(line, "|");
  const items = itemsText ? itemsText.split(";").filter(Boolean).map((item) => {
    const [productId, productName, quantity, priceAtPurchase] = splitLine(item, ",");
    return {
      productId: Number(productId),
      productName,
      quantity: Number(quantity),
      priceAtPurchase: Number(priceAtPurchase)
    };
  }) : [];

  return {
    orderId: Number(orderId),
    username,
    totalPrice: Number(totalPrice),
    timestamp,
    items
  };
}

function serializeOrder(order) {
  const items = order.items.map((item) => [
    item.productId,
    String(item.productName).replaceAll(",", " "),
    item.quantity,
    Number(item.priceAtPurchase).toFixed(6)
  ].join(",")).join(";");

  return [
    order.orderId,
    order.username,
    Number(order.totalPrice).toFixed(6),
    order.timestamp,
    items
  ].join("|");
}

async function getOrders() {
  return (await readLines(files.orders)).map(parseOrder).filter((order) => order.orderId);
}

async function saveOrders(orders) {
  await writeLines(files.orders, orders.map(serializeOrder));
}

function getSession(request) {
  const cookie = request.headers.cookie || "";
  const match = cookie.match(/(?:^|;\s*)session=([^;]+)/);
  return match ? sessions.get(match[1]) : null;
}

async function readJson(request) {
  const chunks = [];
  for await (const chunk of request) {
    chunks.push(chunk);
  }
  const body = Buffer.concat(chunks).toString("utf8");
  return body ? JSON.parse(body) : {};
}

function sendJson(response, status, payload, headers = {}) {
  response.writeHead(status, {
    "Content-Type": "application/json; charset=utf-8",
    ...headers
  });
  response.end(JSON.stringify(payload));
}

function sendError(response, status, message) {
  sendJson(response, status, { error: message });
}

function requireUser(request, response) {
  const user = getSession(request);
  if (!user) {
    sendError(response, 401, "Please sign in first.");
    return null;
  }
  return user;
}

function requireAdmin(request, response) {
  const user = requireUser(request, response);
  if (!user) return null;
  if (user.role !== "admin") {
    sendError(response, 403, "Admin access is required.");
    return null;
  }
  return user;
}

async function handleApi(request, response, pathname) {
  if (request.method === "POST" && pathname === "/api/login") {
    const { username, password } = await readJson(request);
    const users = await getUsers();
    const user = users.find((item) => item.username === username && item.password === password);

    if (!user) {
      sendError(response, 401, "Invalid username or password.");
      return;
    }

    createSession(response, user);
    return;
  }

  if (request.method === "POST" && pathname === "/api/register") {
    const { username, password } = await readJson(request);
    const cleanUsername = String(username || "").trim();
    const cleanPassword = String(password || "").trim();

    if (!/^[A-Za-z0-9_]{3,20}$/.test(cleanUsername)) {
      sendError(response, 400, "Username must be 3-20 letters, numbers, or underscores.");
      return;
    }

    if (cleanPassword.length < 4) {
      sendError(response, 400, "Password must be at least 4 characters.");
      return;
    }

    const users = await getUsers();
    if (users.some((user) => user.username.toLowerCase() === cleanUsername.toLowerCase())) {
      sendError(response, 409, "That username is already taken.");
      return;
    }

    const user = { username: cleanUsername, password: cleanPassword, role: "customer" };
    users.push(user);
    await saveUsers(users);
    createSession(response, user);
    return;
  }

  if (request.method === "POST" && pathname === "/api/guest") {
    createSession(response, { username: "guest", role: "guest" });
    return;
  }

  if (request.method === "POST" && pathname === "/api/logout") {
    const cookie = request.headers.cookie || "";
    const match = cookie.match(/(?:^|;\s*)session=([^;]+)/);
    if (match) sessions.delete(match[1]);
    sendJson(response, 200, { ok: true }, {
      "Set-Cookie": "session=; HttpOnly; SameSite=Lax; Path=/; Max-Age=0"
    });
    return;
  }

  if (request.method === "GET" && pathname === "/api/me") {
    sendJson(response, 200, { user: getSession(request) });
    return;
  }

  if (request.method === "GET" && pathname === "/api/products") {
    sendJson(response, 200, { products: await getProducts() });
    return;
  }

  if (request.method === "POST" && pathname === "/api/products") {
    if (!requireAdmin(request, response)) return;
    const body = await readJson(request);
    const name = String(body.name || "").trim();
    const category = String(body.category || "").trim();
    const price = Number(body.price);

    if (!name || !category || !Number.isFinite(price) || price < 0) {
      sendError(response, 400, "Product name, category, and a valid price are required.");
      return;
    }

    const products = await getProducts();
    const nextId = products.reduce((max, product) => Math.max(max, product.id), 0) + 1;
    const product = { id: nextId, name, category, price, views: 0, purchases: 0 };
    products.push(product);
    await saveProducts(products);
    sendJson(response, 201, { product });
    return;
  }

  const productMatch = pathname.match(/^\/api\/products\/(\d+)$/);
  if (request.method === "PUT" && productMatch) {
    if (!requireAdmin(request, response)) return;
    const id = Number(productMatch[1]);
    const body = await readJson(request);
    const products = await getProducts();
    const product = products.find((item) => item.id === id);

    if (!product) {
      sendError(response, 404, "Product was not found.");
      return;
    }

    product.name = String(body.name || product.name).trim();
    product.category = String(body.category || product.category).trim();
    product.price = Number.isFinite(Number(body.price)) ? Number(body.price) : product.price;
    await saveProducts(products);
    sendJson(response, 200, { product });
    return;
  }

  if (request.method === "DELETE" && productMatch) {
    if (!requireAdmin(request, response)) return;
    const id = Number(productMatch[1]);
    const products = await getProducts();
    const remaining = products.filter((product) => product.id !== id);
    await saveProducts(remaining);
    sendJson(response, 200, { ok: true });
    return;
  }

  const viewMatch = pathname.match(/^\/api\/products\/(\d+)\/view$/);
  if (request.method === "POST" && viewMatch) {
    const id = Number(viewMatch[1]);
    const user = getSession(request);
    const products = await getProducts();
    const product = products.find((item) => item.id === id);
    if (!product) {
      sendError(response, 404, "Product was not found.");
      return;
    }
    product.views += 1;
    await saveProducts(products);
    if (user) {
      await updateInteraction(user.username, id, "views", 1);
    }
    sendJson(response, 200, { product });
    return;
  }

  if (request.method === "GET" && pathname === "/api/recommendations") {
    const user = getSession(request);
    const products = await getProducts();
    const interactions = user
      ? (await getInteractions()).filter((item) => item.username === user.username)
      : [];
    const byProduct = new Map(interactions.map((item) => [item.productId, item]));
    const categoryCounts = new Map();

    for (const interaction of interactions) {
      const product = products.find((item) => item.id === interaction.productId);
      if (!product) continue;
      categoryCounts.set(
        product.category,
        (categoryCounts.get(product.category) || 0) + interaction.views + interaction.purchases
      );
    }

    let favoriteCategory = "";
    let favoriteCount = 0;
    for (const [category, count] of categoryCounts) {
      if (count > favoriteCount) {
        favoriteCategory = category;
        favoriteCount = count;
      }
    }

    const recommendations = [...products]
      .map((product) => {
        const interaction = byProduct.get(product.id) || { views: 0, purchases: 0 };
        const categoryBonus = favoriteCategory && product.category === favoriteCategory ? 5 : 0;
        return {
          ...product,
          score: interaction.views * 0.2 + interaction.purchases * 0.8 + categoryBonus
        };
      })
      .sort((a, b) => {
        if (a.score !== b.score) return b.score - a.score;
        if (a.purchases !== b.purchases) return b.purchases - a.purchases;
        return b.views - a.views;
      })
      .slice(0, 4);
    sendJson(response, 200, { recommendations });
    return;
  }

  if (request.method === "GET" && pathname === "/api/orders") {
    const user = requireUser(request, response);
    if (!user) return;
    const orders = await getOrders();
    sendJson(response, 200, {
      orders: user.role === "admin" ? orders : orders.filter((order) => order.username === user.username)
    });
    return;
  }

  if (request.method === "POST" && pathname === "/api/orders") {
    const user = requireUser(request, response);
    if (!user) return;
    const { items = [] } = await readJson(request);
    const products = await getProducts();
    const orderItems = [];

    for (const item of items) {
      const product = products.find((entry) => entry.id === Number(item.productId));
      const quantity = Number(item.quantity);
      if (product && Number.isInteger(quantity) && quantity > 0) {
        product.purchases += quantity;
        orderItems.push({
          productId: product.id,
          productName: product.name,
          quantity,
          priceAtPurchase: product.price
        });
      }
    }

    if (!orderItems.length) {
      sendError(response, 400, "Your cart is empty.");
      return;
    }

    const orders = await getOrders();
    const order = {
      orderId: orders.reduce((max, item) => Math.max(max, item.orderId), 0) + 1,
      username: user.username,
      totalPrice: orderItems.reduce((sum, item) => sum + item.quantity * item.priceAtPurchase, 0),
      timestamp: new Date().toISOString(),
      items: orderItems
    };

    orders.push(order);
    await Promise.all([
      saveOrders(orders),
      saveProducts(products),
      ...orderItems.map((item) => updateInteraction(user.username, item.productId, "purchases", item.quantity))
    ]);
    sendJson(response, 201, { order });
    return;
  }

  sendError(response, 404, "API route was not found.");
}

async function serveStatic(response, pathname) {
  const safePath = pathname === "/" ? "/index.html" : pathname;
  const filePath = path.normalize(path.join(publicDir, safePath));

  if (!filePath.startsWith(publicDir)) {
    response.writeHead(403);
    response.end("Forbidden");
    return;
  }

  try {
    const data = await fs.readFile(filePath);
    const type = contentTypes[path.extname(filePath)] || "application/octet-stream";
    response.writeHead(200, { "Content-Type": type });
    response.end(data);
  } catch {
    response.writeHead(404, { "Content-Type": "text/plain; charset=utf-8" });
    response.end("Not found");
  }
}

async function handleRequest(request, response) {
  try {
    const url = new URL(request.url, `http://${request.headers.host}`);
    if (url.pathname.startsWith("/api/")) {
      await handleApi(request, response, url.pathname);
      return;
    }
    await serveStatic(response, url.pathname);
  } catch (error) {
    console.error(error);
    sendError(response, 500, "Something went wrong on the server.");
  }
}

ensureDataFiles().then(() => {
  http.createServer(handleRequest).listen(port, () => {
    console.log(`Smart Ecommerce web app is running at http://localhost:${port}`);
  });
});
