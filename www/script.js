document.addEventListener("DOMContentLoaded", function () {
    const title = document.querySelector("h1");
    const links = document.querySelectorAll("a");

    // Effetto dissolvenza sul titolo
    title.style.opacity = 0;
    setTimeout(() => {
        title.style.transition = "opacity 2s";
        title.style.opacity = 1;
    }, 500);

    // Animazione sui link
    links.forEach(link => {
        link.addEventListener("mouseover", () => {
            link.style.transform = "scale(1.1)";
            link.style.transition = "transform 0.3s";
        });
        link.addEventListener("mouseout", () => {
            link.style.transform = "scale(1)";
        });
    });
});

