const container = document.querySelector('#container');
const signInButton = document.querySelector('#signIn');
const signUpButton = document.querySelector('#signUp');
const signUpForm = document.querySelector('#signUpForm');
const signInForm = document.querySelector('#signInForm');

// Object to store user credentials
let users = {};

signUpButton.addEventListener('click', () => container.classList.add('right-panel-active'));
signInButton.addEventListener('click', () => container.classList.remove('right-panel-active'));

signUpForm.addEventListener('submit', (event) => {
  event.preventDefault();
  const username = document.querySelector('#signUpUsername').value;
  const password = document.querySelector('#signUpPassword').value;
  if (users[username]) {
    alert('This username is already registered. Please use another username.');
  } else {
    users[username] = password;
    alert('Registration successful! You can now log in.');
  }
});

signInForm.addEventListener('submit', (event) => {
  event.preventDefault();
  const username = document.querySelector('#signInUsername').value;
  const password = document.querySelector('#signInPassword').value;
  if (users[username] && users[username] === password) {
    alert('Login successful!');
  } else {
    alert('Invalid username or password. Please try again.');
  }
});