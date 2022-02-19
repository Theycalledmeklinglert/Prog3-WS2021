import { Component, Input } from '@angular/core';
import { Item } from 'src/lib/data-access/modelClasses/Item';

@Component({
  selector: 'column',
  templateUrl: './column.component.html',
  styleUrls: ['./column.component.scss'],
})
export class ColumnComponent {
  @Input()
  title = 'column'; // Warun hier '' anstatt "" ?

  @Input()
  id : number;

  @Input()
  items: Item[
    {
      id: 0,
      title: "first task",
      position: 0,
      timestamp: "11-12-2021",
    },
    {
      id: 1,
      title: "second task",
      position: 1,
      timestamp: "11-12-2021",
    },
    {
      id: 2,
      title: "third task",
      position: 2,
      timestamp: "11-12-2021",
    },
    ];
 }
