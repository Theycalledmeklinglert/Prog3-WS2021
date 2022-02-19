import { Item } from "./Item";

export interface Column {
  id:number;
  title:string;
  position:number;
  items: Item[];  // Wie kann ich die Items hier angeben? --> Ueber import der Modelklasse
}
